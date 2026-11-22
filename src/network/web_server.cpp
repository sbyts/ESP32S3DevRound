#include "web_server.h"
#include "app_config.h"
#include "web_ui.h"
#include "../system/sys_logger.h"
#include <WiFi.h>
#include <ArduinoJson.h>

WebAdminServer webAdmin;

String WebAdminServer::formatUptime(uint32_t sec)
{
    uint32_t days = sec / 86400;
    uint32_t hrs  = (sec / 3600) % 24;
    uint32_t mins = (sec / 60) % 60;
    uint32_t secs = sec % 60;

    String res = String(days) + "d ";
    res += (hrs < 10 ? "0" : "") + String(hrs) + ":";
    res += (mins < 10 ? "0" : "") + String(mins) + ":";
    res += (secs < 10 ? "0" : "") + String(secs);
    return res;
}

bool WebAdminServer::checkAuth(AsyncWebServerRequest *request)
{
    if (!request->authenticate("admin", appConfig.adminPass.c_str()))
    {
        request->requestAuthentication("Climate Core OS");
        return false;
    }
    return true;
}

void WebAdminServer::begin()
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
        request->send(response);
    });

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
        res->printf("\"boots\":%u,", appConfig.bootCount);
        res->print("\"logs\":[");

        uint8_t cnt = sys_log.getCount();
        for (uint8_t i = 0; i < cnt; i++)
        {
            LogEntry entry = sys_log.getEntry(i);
            String clean_txt = entry.text;
            clean_txt.replace("\\", "\\\\");
            clean_txt.replace("\"", "'");
            res->printf("{\"txt\":\"%s\",\"lvl\":%d}", clean_txt.c_str(), (int)entry.level);
            if (i + 1 < cnt) res->print(",");
        }
        res->print("]}");
        request->send(res);
    });

    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request)
    {
        if (!checkAuth(request)) return;

        AsyncResponseStream *res = request->beginResponseStream("application/json");
        String clean_city = appConfig.city;
        clean_city.replace("\"", "'");
        String clean_lamp_url = appConfig.lampUrl;
        clean_lamp_url.replace("\"", "'");
        String clean_lamp_key = appConfig.lampKey;
        clean_lamp_key.replace("\"", "'");

        res->print("{");
        res->printf("\"city\":\"%s\",", clean_city.c_str());
        res->printf("\"radar_en\":%s,", appConfig.radarEnabled ? "true" : "false");
        res->printf("\"eco_wifi\":%s,", appConfig.ecoWifiOff ? "true" : "false");
        res->printf("\"lamp_url\":\"%s\",", clean_lamp_url.c_str());
        res->printf("\"lamp_key\":\"%s\",", clean_lamp_key.c_str());
        res->printf("\"lamp_delay\":%u,", appConfig.lampDelaySec);
        res->printf("\"lamp_auto\":%s,", appConfig.lampAutoPresence ? "true" : "false");
        res->print("\"networks\":[");

        for (uint8_t i = 0; i < appConfig.netCount; i++)
        {
            String clean_ssid = appConfig.networks[i].ssid;
            clean_ssid.replace("\"", "'");
            res->printf("{\"ssid\":\"%s\"}", clean_ssid.c_str());
            if (i + 1 < appConfig.netCount) res->print(",");
        }
        res->print("]}");
        request->send(res);
    });

    _server.on("/api/save", HTTP_POST, [this](AsyncWebServerRequest *request)
    {
        if (!checkAuth(request)) return;
        request->send(200, "text/plain", "OK");
    }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    {
        if (!request->authenticate("admin", appConfig.adminPass.c_str())) return;

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (!err)
        {
            const char* new_city = doc["city"] | "";
            if (strlen(new_city) > 0)
            {
                appConfig.setCity(new_city);
            }

            const char* new_pass = doc["admin_pass"] | "";
            if (strlen(new_pass) > 0)
            {
                appConfig.setAdminPass(new_pass);
            }

            if (!doc["radar_en"].isNull())
            {
                appConfig.setRadarEnabled(doc["radar_en"] | false);
            }

            if (!doc["eco_wifi"].isNull())
            {
                appConfig.setEcoWifiOff(doc["eco_wifi"] | false);
            }

            const char* new_lamp_url = doc["lamp_url"] | "";
            if (strlen(new_lamp_url) > 0)
            {
                appConfig.setLampUrl(new_lamp_url);
            }

            const char* new_lamp_key = doc["lamp_key"] | "";
            if (strlen(new_lamp_key) > 0)
            {
                appConfig.setLampKey(new_lamp_key);
            }

            if (!doc["lamp_delay"].isNull())
            {
                appConfig.setLampDelay(doc["lamp_delay"] | DEFAULT_LAMP_DELAY);
            }

            if (!doc["lamp_auto"].isNull())
            {
                appConfig.setLampAuto(doc["lamp_auto"] | true);
            }

            if (doc["networks"].is<JsonArray>())
            {
                JsonArray arr = doc["networks"].as<JsonArray>();
                WifiCreds updated_nets[AppConfig::MAX_WIFI_NETWORKS];
                uint8_t updated_count = 0;

                for (JsonObject n : arr)
                {
                    if (updated_count < AppConfig::MAX_WIFI_NETWORKS)
                    {
                        String s = String(n["ssid"] | "");
                        String p = String(n["pass"] | "");
                        if (s.length() > 0)
                        {
                            updated_nets[updated_count].ssid = s;
                            if (p.length() > 0)
                            {
                                updated_nets[updated_count].pass = p;
                            }
                            else
                            {
                                String existing_pass = "";
                                for (uint8_t k = 0; k < appConfig.netCount; k++)
                                {
                                    if (appConfig.networks[k].ssid == s)
                                    {
                                        existing_pass = appConfig.networks[k].pass;
                                        break;
                                    }
                                }
                                updated_nets[updated_count].pass = existing_pass;
                            }
                            updated_count++;
                        }
                    }
                }

                appConfig.netCount = updated_count;
                for (uint8_t i = 0; i < appConfig.netCount; i++)
                {
                    appConfig.networks[i] = updated_nets[i];
                }
                appConfig.saveWiFiNetworks();
            }

            sys_log.log("Web config saved. Rebooting...", LOG_NET);
            delay(500);
            ESP.restart();
        }
    });

    _server.begin();
    sys_log.log("Web Server started (Auth ON)", LOG_INFO);
}