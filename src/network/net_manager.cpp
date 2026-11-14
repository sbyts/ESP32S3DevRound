#include "net_manager.h"
#include "web_ui.h"
#include "../display/ui_manager.h"
#include "esp_sntp.h"

NetManager net_service;

void NetManager::init() {
    _prefs.begin("climate_cfg", false);
    _ssid = _prefs.getString("ssid", "");
    _pass = _prefs.getString("pass", "");
    _city = _prefs.getString("city", "Kyiv");

    String upper_city = _city;
    upper_city.toUpperCase();
    ui.setOutdoorData(18.0f, 75.0f, 0.0f, "CLOUDY", upper_city.c_str());

    if (_ssid.length() > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.setAutoReconnect(true);
        WiFi.begin(_ssid.c_str(), _pass.c_str());
        
        uint8_t attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        _is_connected = true;
        sntp_set_sync_interval(300000);
        configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org", "time.nist.gov");
        
        fetchSimpleWeather();
    } else {
        _is_connected = false;
        _weather_status = "NO WI-FI";
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ClimateStation-AP");
    }

    setupWebServer();
}

void NetManager::setupWebServer() {
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", INDEX_HTML);
    });

    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["ssid"] = _ssid;
        doc["city"] = _city;
        doc["ip"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    _server.on("/api/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true)) _prefs.putString("ssid", request->getParam("ssid", true)->value());
        if (request->hasParam("pass", true)) _prefs.putString("pass", request->getParam("pass", true)->value());
        if (request->hasParam("city", true)) _prefs.putString("city", request->getParam("city", true)->value());
        
        request->send(200, "text/plain", "OK");
        delay(500);
        ESP.restart();
    });

    _server.begin();
}

void NetManager::fetchSimpleWeather() {
    if (WiFi.status() != WL_CONNECTED) {
        _weather_status = "OFFLINE";
        return;
    }

    HTTPClient http;
    http.setTimeout(3000);
    http.setReuse(false);
    http.setUserAgent("curl/7.68.0");

    String clean_city = _city;
    clean_city.replace(" ", "+");
    
    // Format: temperature | humidity | wind | condition (e.g. "+8°C|92%|14km/h|Light rain")
    String uri = "/" + clean_city + "?format=%t|%h|%w|%C";

    if (http.begin("wttr.in", 80, uri)) {
        int httpCode = http.GET();
        if (httpCode == 200) {
            String payload = http.getString();
            payload.trim();
            http.end();

            int sep1 = payload.indexOf('|');
            int sep2 = payload.indexOf('|', sep1 + 1);
            int sep3 = payload.indexOf('|', sep2 + 1);
            
            if (sep1 > 0 && sep2 > sep1 && sep3 > sep2) {
                String temp_str = payload.substring(0, sep1);
                String hum_str  = payload.substring(sep1 + 1, sep2);
                String wind_str = payload.substring(sep2 + 1, sep3);
                String cond_str = payload.substring(sep3 + 1);
                cond_str.trim();
                
                temp_str.replace("+", "");
                temp_str.replace("°C", "");
                hum_str.replace("%", "");

                String clean_wind = "";
                for (size_t i = 0; i < wind_str.length(); i++) {
                    char c = wind_str.charAt(i);
                    if (isDigit(c) || c == '.') {
                        clean_wind += c;
                    }
                }
                
                float out_temp = temp_str.toFloat();
                float out_hum  = hum_str.toFloat();
                float wind_ms  = clean_wind.toFloat() / 3.6f;

                String upper_city = _city;
                upper_city.toUpperCase();
                
                ui.setOutdoorData(out_temp, out_hum, wind_ms, cond_str.c_str(), upper_city.c_str());
                _weather_status = "OK: " + cond_str + " (" + String((int)round(out_temp)) + "C)";
            } else {
                _weather_status = "BAD FORMAT";
            }
        } else {
            _weather_status = "HTTP ERR: " + String(httpCode);
            http.end();
        }
    } else {
        _weather_status = "CONN FAILED";
    }
    _last_weather_update = millis();
}

void NetManager::process() {
    if (WiFi.status() == WL_CONNECTED) {
        if (millis() - _last_weather_update > WEATHER_INTERVAL) {
            fetchSimpleWeather();
        }
    }
}