#include "net_manager.h"
#include "web_ui.h"
#include "../display/ui_manager.h"
#include "../peripherals/radar_ld2410.h"
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

    // Disabled by default so the screen stays ON until enabled via Web UI
    _radar_enabled = _prefs.getBool("radar_en", false);
    _eco_wifi_off = _prefs.getBool("eco_wifi", false);
    _wifi_suspended = false;

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

    // Migrate legacy single-SSID setting safely
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
    radar.init(_radar_enabled, _eco_wifi_off);

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

void NetManager::setEcoMode(bool enable_eco, bool deep_wifi_off)
{
    if (enable_eco)
    {
        if (deep_wifi_off)
        {
            if (!_wifi_suspended && _is_connected)
            {
                _wifi_suspended = true;
                WiFi.disconnect(true);
                WiFi.mode(WIFI_OFF);
            }
        }
        else
        {
            if (_is_connected)
            {
                WiFi.setSleep(true);
            }
        }
    }
    else
    {
        if (_wifi_suspended)
        {
            _wifi_suspended = false;
            if (connectToBestAvailableWiFi())
            {
                _is_connected = true;
                sys_log.log("Wi-Fi restored: " + WiFi.localIP().toString(), LOG_NET);
            }
        }
        else if (_is_connected)
        {
            WiFi.setSleep(false);
        }
    }
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
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print(FPSTR(WEB_HTML_HEAD));
        response->print(FPSTR(WEB_CSS));
        response->print(FPSTR(WEB_HTML_BODY));
        response->print(FPSTR(WEB_JS));
        response->print(FPSTR(WEB_HTML_FOOT));
        request->send(response); });

    _server.on("/api/telemetry", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
        if (!checkAuth(request)) return;

        AsyncResponseStream *res = request->beginResponseStream("application/json");
        String ip_str = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
        uint32_t total_kb = ESP.getHeapSize() / 1024;
        uint32_t free_kb  = ESP.getFreeHeap() / 1024;

        res->print("{");
        res->printf("\"ip\":\"%s\",", ip_str.c_str());
        res->printf("\"uptime\":\"%s\",", formatUptime(millis() / 1000).c_str());
        res->printf("\"total_kb\":%u,", total_kb);
        res->printf("\"free_kb\":%u,", free_kb);
        res->printf("\"used_kb\":%u,", total_kb - free_kb);
        res->printf("\"min_kb\":%u,", ESP.getMinFreeHeap() / 1024);
        res->printf("\"cpu\":%u,", ESP.getCpuFreqMHz());
        res->printf("\"temp\":\"%.1f\",", temperatureRead());
        res->printf("\"rssi\":%d,", (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0);
        res->printf("\"boots\":%u,", _boot_count);
        res->print("\"logs\":[");

        uint8_t cnt = sys_log.getCount();
        for (uint8_t i = 0; i < cnt; i++) {
            LogEntry entry = sys_log.getEntry(i);
            String clean_txt = entry.text;
            clean_txt.replace("\\", "\\\\");
            clean_txt.replace("\"", "'");
            res->printf("{\"txt\":\"%s\",\"lvl\":%d}", clean_txt.c_str(), (int)entry.level);
            if (i + 1 < cnt) res->print(",");
        }
        res->print("]}");
        request->send(res); });

    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
        if (!checkAuth(request)) return;

        AsyncResponseStream *res = request->beginResponseStream("application/json");
        String clean_city = _city;
        clean_city.replace("\"", "'");

        res->print("{");
        res->printf("\"city\":\"%s\",", clean_city.c_str());
        res->printf("\"radar_en\":%s,", _radar_enabled ? "true" : "false");
        res->printf("\"eco_wifi\":%s,", _eco_wifi_off ? "true" : "false");
        res->print("\"networks\":[");

        for (uint8_t i = 0; i < _net_count; i++) {
            String clean_ssid = _networks[i].ssid;
            clean_ssid.replace("\"", "'");
            res->printf("{\"ssid\":\"%s\"}", clean_ssid.c_str());
            if (i + 1 < _net_count) res->print(",");
        }
        res->print("]}");
        request->send(res); });

    _server.on("/api/save", HTTP_POST, [this](AsyncWebServerRequest *request)
               {
        if (!checkAuth(request)) return;
        request->send(200, "text/plain", "OK"); }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        if (!request->authenticate("admin", _admin_pass.c_str())) return;

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (!err) {
            const char* new_city = doc["city"] | "";
            if (strlen(new_city) > 0) {
                _prefs.putString("city", new_city);
            }

            const char* new_pass = doc["admin_pass"] | "";
            if (strlen(new_pass) > 0) {
                _prefs.putString("admin_pass", new_pass);
            }

            if (!doc["radar_en"].isNull()) {
                _radar_enabled = doc["radar_en"] | false;
                _prefs.putBool("radar_en", _radar_enabled);
            }

            if (!doc["eco_wifi"].isNull()) {
                _eco_wifi_off = doc["eco_wifi"] | false;
                _prefs.putBool("eco_wifi", _eco_wifi_off);
            }

            if (doc["networks"].is<JsonArray>()) {
                JsonArray arr = doc["networks"].as<JsonArray>();
                WifiCreds updated_nets[MAX_WIFI_NETWORKS];
                uint8_t updated_count = 0;

                for (JsonObject n : arr) {
                    if (updated_count < MAX_WIFI_NETWORKS) {
                        String s = String(n["ssid"] | "");
                        String p = String(n["pass"] | "");
                        if (s.length() > 0) {
                            updated_nets[updated_count].ssid = s;
                            if (p.length() > 0) {
                                updated_nets[updated_count].pass = p;
                            } else {
                                String existing_pass = "";
                                for (uint8_t k = 0; k < _net_count; k++) {
                                    if (_networks[k].ssid == s) {
                                        existing_pass = _networks[k].pass;
                                        break;
                                    }
                                }
                                updated_nets[updated_count].pass = existing_pass;
                            }
                            updated_count++;
                        }
                    }
                }

                _net_count = updated_count;
                for (uint8_t i = 0; i < _net_count; i++) {
                    _networks[i] = updated_nets[i];
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
void NetManager::fetchSimpleWeather()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        _weather_status = "OFFLINE";
        return;
    }
    HTTPClient http;
    http.setTimeout(3000);
    http.setReuse(false);
    http.setUserAgent("curl/7.68.0");
    String clean_city = _city;
    clean_city.replace(" ", "+");
    // Force metric units (?m) so wttr.in always returns Celsius and km/h
    String uri = "/" + clean_city + "?m&format=%t|%h|%w|%C";
    if (http.begin("wttr.in", 80, uri))
    {
        int httpCode = http.GET();
        if (httpCode == 200)
        {
            String payload = http.getString();
            payload.trim();
            http.end();
            int sep1 = payload.indexOf('|');
            int sep2 = payload.indexOf('|', sep1 + 1);
            int sep3 = payload.indexOf('|', sep2 + 1);
            if (sep1 > 0 && sep2 > sep1 && sep3 > sep2)
            {
                String temp_str = payload.substring(0, sep1);
                String hum_str = payload.substring(sep1 + 1, sep2);
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
                for (size_t i = 0; i < wind_str.length(); i++)
                {
                    char c = wind_str.charAt(i);
                    if (isDigit(c) || c == '.')
                        clean_wind += c;
                }
                float out_temp = temp_str.toFloat();
                if (is_fahrenheit)
                {
                    out_temp = (out_temp - 32.0f) * 5.0f / 9.0f;
                }
                float out_hum = hum_str.toFloat();
                float wind_ms = clean_wind.toFloat() / 3.6f;
                String upper_city = _city;
                upper_city.toUpperCase();
                ui.setOutdoorData(out_temp, out_hum, wind_ms, cond_str.c_str(), upper_city.c_str());
                _weather_status = "OK: " + cond_str + " (" + String((int)round(out_temp)) + "C)";
                sys_log.log("Weather: " + upper_city + " " + String((int)round(out_temp)) + "C", LOG_NET);
            }
            else
            {
                _weather_status = "BAD FORMAT";
                sys_log.log("Weather parse error", LOG_ERR);
            }
        }
        else
        {
            _weather_status = "HTTP ERR: " + String(httpCode);
            sys_log.log("Weather HTTP " + String(httpCode), LOG_ERR);
            http.end();
        }
    }
    else
    {
        _weather_status = "CONN FAILED";
        sys_log.log("Weather conn failed", LOG_ERR);
    }
    _last_weather_update = millis();
}
void NetManager::process()
{
    if (WiFi.status() == WL_CONNECTED && !_wifi_suspended)
    {
        if (millis() - _last_weather_update > WEATHER_INTERVAL)
        {
            fetchSimpleWeather();
        }
    }
}