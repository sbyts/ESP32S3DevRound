#include "net_manager.h"
#include "web_ui.h"
#include "../display/ui_manager.h"
#include "../system/sys_logger.h"
#include "esp_sntp.h"

NetManager net_service;

static String formatUptime(uint32_t sec)
{
    uint32_t days = sec / 86400;
    uint32_t hrs = (sec / 3600) % 24;
    uint32_t mins = (sec / 60) % 60;
    uint32_t secs = sec % 60;

    String res = String(days) + "d ";
    res += (hrs < 10 ? "0" : "") + String(hrs) + ":";
    res += (mins < 10 ? "0" : "") + String(mins) + ":";
    res += (secs < 10 ? "0" : "") + String(secs);
    return res;
}

void NetManager::loadSettings()
{
    _prefs.begin("climate_cfg", false);

    _boot_count = _prefs.getUInt("boots", 0) + 1;
    _prefs.putUInt("boots", _boot_count);

    _city = _prefs.getString("city", "Kyiv");
    _admin_pass = _prefs.getString("admin_pass", "admin");

    _net_count = 0;
    for (uint8_t i = 0; i < MAX_WIFI_NETWORKS; i++)
    {
        String s_key = "ssid_" + String(i);
        String p_key = "pass_" + String(i);
        String s_val = _prefs.getString(s_key.c_str(), "");
        if (s_val.length() > 0)
        {
            _networks[_net_count].ssid = s_val;
            _networks[_net_count].pass = _prefs.getString(p_key.c_str(), "");
            _net_count++;
        }
    }

    // Migrate old single-SSID setting safely without square brackets
    if (_net_count == 0)
    {
        String legacy_ssid = _prefs.getString("ssid", "");
        if (legacy_ssid.length() > 0)
        {
            (*_networks).ssid = legacy_ssid;
            (*_networks).pass = _prefs.getString("pass", "");
            _net_count = 1;
            saveNetworksToPrefs();
        }
    }
}

void NetManager::saveNetworksToPrefs()
{
    for (uint8_t i = 0; i < MAX_WIFI_NETWORKS; i++)
    {
        String s_key = "ssid_" + String(i);
        String p_key = "pass_" + String(i);
        if (i < _net_count)
        {
            _prefs.putString(s_key.c_str(), _networks[i].ssid);
            _prefs.putString(p_key.c_str(), _networks[i].pass);
        }
        else
        {
            _prefs.remove(s_key.c_str());
            _prefs.remove(p_key.c_str());
        }
    }
}

bool NetManager::connectToBestAvailableWiFi()
{
    if (_net_count == 0)
    {
        sys_log.log("No saved Wi-Fi networks", LOG_ERR);
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);

    sys_log.log("Scanning Wi-Fi networks...", LOG_NET);
    int found = WiFi.scanNetworks();

    int best_idx = -1;
    int best_rssi = -999;

    for (int i = 0; i < found; i++)
    {
        String scanned_ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);
        for (uint8_t j = 0; j < _net_count; j++)
        {
            if (scanned_ssid == _networks[j].ssid && rssi > best_rssi)
            {
                best_rssi = rssi;
                best_idx = j;
            }
        }
    }
    WiFi.scanDelete();

    if (best_idx >= 0)
    {
        sys_log.log("Connecting: " + _networks[best_idx].ssid, LOG_NET);
        WiFi.begin(_networks[best_idx].ssid.c_str(), _networks[best_idx].pass.c_str());

        uint8_t attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20)
        {
            delay(500);
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            return true;
        }
    }

    sys_log.log("Saved Wi-Fi unreachable", LOG_ERR);
    return false;
}

void NetManager::init()
{
    loadSettings();

    String upper_city = _city;
    upper_city.toUpperCase();
    ui.setOutdoorData(18.0f, 75.0f, 0.0f, "CLOUDY", upper_city.c_str());

    if (connectToBestAvailableWiFi())
    {
        _is_connected = true;
        sys_log.log("IP: " + WiFi.localIP().toString(), LOG_NET);

        sntp_set_sync_interval(300000);
        configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org", "time.nist.gov");
        sys_log.log("NTP time sync started", LOG_INFO);

        fetchSimpleWeather();
    }
    else
    {
        _is_connected = false;
        _weather_status = "AP MODE";
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ClimateStation-AP");
        sys_log.log("AP Started: 192.168.4.1", LOG_ERR);
    }

    setupWebServer();
}

bool NetManager::checkAuth(AsyncWebServerRequest *request)
{
    if (!request->authenticate("admin", _admin_pass.c_str()))
    {
        request->requestAuthentication("Climate Core OS");
        return false;
    }
    return true;
}

void NetManager::setupWebServer()
{
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
        if (!checkAuth(request)) return;
        request->send(200, "text/html", INDEX_HTML); });

    _server.on("/api/telemetry", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
        if (!checkAuth(request)) return;

        JsonDocument doc;
        doc["ip"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
        doc["uptime"] = formatUptime(millis() / 1000);

        uint32_t total_kb = ESP.getHeapSize() / 1024;
        uint32_t free_kb  = ESP.getFreeHeap() / 1024;
        doc["total_kb"] = total_kb;
        doc["free_kb"]  = free_kb;
        doc["used_kb"]  = total_kb - free_kb;
        doc["min_kb"]   = ESP.getMinFreeHeap() / 1024;
        doc["cpu"]      = ESP.getCpuFreqMHz();
        doc["temp"]     = String(temperatureRead(), 1);
        doc["rssi"]     = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
        doc["boots"]    = _boot_count;

        JsonArray logs_arr = doc["logs"].to<JsonArray>();
        for (uint8_t i = 0; i < sys_log.getCount(); i++) {
            LogEntry entry = sys_log.getEntry(i);
            JsonObject obj = logs_arr.add<JsonObject>();
            obj["txt"] = entry.text;
            obj["lvl"] = (int)entry.level;
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
        if (!checkAuth(request)) return;

        JsonDocument doc;
        doc["city"] = _city;
        JsonArray nets = doc["networks"].to<JsonArray>();
        for (uint8_t i = 0; i < _net_count; i++) {
            JsonObject n = nets.add<JsonObject>();
            n["ssid"] = _networks[i].ssid;
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    _server.on("/api/save", HTTP_POST, [this](AsyncWebServerRequest *request)
               {
        if (!checkAuth(request)) return;
        request->send(200, "text/plain", "OK"); }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        if (!request->authenticate("admin", _admin_pass.c_str())) return;

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (!err) {
            if (doc["city"].is<String>()) {
                String new_city = doc["city"].as<String>();
                if (new_city.length() > 0) _prefs.putString("city", new_city);
            }
            if (doc["admin_pass"].is<String>()) {
                String new_pass = doc["admin_pass"].as<String>();
                if (new_pass.length() > 0) _prefs.putString("admin_pass", new_pass);
            }
            if (doc["networks"].is<JsonArray>()) {
                JsonArray arr = doc["networks"].as<JsonArray>();
                _net_count = 0;
                for (JsonObject n : arr) {
                    if (_net_count < MAX_WIFI_NETWORKS) {
                        String s = n["ssid"].as<String>();
                        String p = n["pass"].as<String>();
                        if (s.length() > 0) {
                            _networks[_net_count].ssid = s;
                            if (p.length() == 0) {
                                for (uint8_t k = 0; k < MAX_WIFI_NETWORKS; k++) {
                                    if (_prefs.getString(("ssid_" + String(k)).c_str(), "") == s) {
                                        p = _prefs.getString(("pass_" + String(k)).c_str(), "");
                                        break;
                                    }
                                }
                            }
                            _networks[_net_count].pass = p;
                            _net_count++;
                        }
                    }
                }
                saveNetworksToPrefs();
            }
            sys_log.log("Web config saved. Rebooting...", LOG_NET);
            delay(500);
            ESP.restart();
        } });

    _server.begin();
    sys_log.log("Web Server started (Auth ON)", LOG_INFO);
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
    
    // Added 'm' parameter (?m&format=...) to strictly force Metric units (°C, km/h)
    String uri = "/" + clean_city + "?m&format=%t|%h|%w|%C";

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

                bool is_fahrenheit = (temp_str.indexOf('F') >= 0);
                temp_str.replace("+", "");
                temp_str.replace("°C", "");
                temp_str.replace("°F", "");
                temp_str.replace("°", "");
                hum_str.replace("%", "");

                String clean_wind = "";
                for (size_t i = 0; i < wind_str.length(); i++) {
                    char c = wind_str.charAt(i);
                    if (isDigit(c) || c == '.') clean_wind += c;
                }
                
                float out_temp = temp_str.toFloat();
                if (is_fahrenheit) {
                    out_temp = (out_temp - 32.0f) * 5.0f / 9.0f;
                }

                float out_hum  = hum_str.toFloat();
                float wind_ms  = clean_wind.toFloat() / 3.6f;

                String upper_city = _city;
                upper_city.toUpperCase();
                
                ui.setOutdoorData(out_temp, out_hum, wind_ms, cond_str.c_str(), upper_city.c_str());
                _weather_status = "OK: " + cond_str + " (" + String((int)round(out_temp)) + "C)";
                sys_log.log("Weather: " + upper_city + " " + String((int)round(out_temp)) + "C", LOG_NET);
            } else {
                _weather_status = "BAD FORMAT";
                sys_log.log("Weather parse error", LOG_ERR);
            }
        } else {
            _weather_status = "HTTP ERR: " + String(httpCode);
            sys_log.log("Weather HTTP " + String(httpCode), LOG_ERR);
            http.end();
        }
    } else {
        _weather_status = "CONN FAILED";
        sys_log.log("Weather conn failed", LOG_ERR);
    }
    _last_weather_update = millis();
}

void NetManager::process()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        if (millis() - _last_weather_update > WEATHER_INTERVAL)
        {
            fetchSimpleWeather();
        }
    }
}