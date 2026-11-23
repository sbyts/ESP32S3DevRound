#include "net_manager.h"
#include "web_server.h"
#include "lamp_client.h"
#include "../display/ui_manager.h"
#include "../peripherals/radar_ld2410.h"
#include "../system/sys_logger.h"
#include "esp_sntp.h"
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

NetManager net_service;

void NetManager::setupOTA()
{
    if (_ota_initialized)
        return;

    ArduinoOTA.setHostname("climate-station");
    ArduinoOTA.setPassword(appConfig.adminPass.c_str());

    ArduinoOTA.onStart([]()
                       { sys_log.log("OTA update started...", LOG_NET); });

    ArduinoOTA.onEnd([]()
                     { sys_log.log("OTA complete! Rebooting...", LOG_NET); });

    ArduinoOTA.onError([](ota_error_t error)
                       { sys_log.log("OTA Error [" + String(error) + "]", LOG_ERR); });

    ArduinoOTA.begin();
    _ota_initialized = true;
    sys_log.log("ArduinoOTA ready (climate-station.local)", LOG_INFO);
}

bool NetManager::connectToBestAvailableWiFi()
{
    if (appConfig.netCount == 0)
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
        for (uint8_t j = 0; j < appConfig.netCount; j++)
        {
            if (scanned_ssid == appConfig.networks[j].ssid && rssi > best_rssi)
            {
                best_rssi = rssi;
                best_idx = j;
            }
        }
    }
    WiFi.scanDelete();

    if (best_idx >= 0)
    {
        sys_log.log("Connecting: " + appConfig.networks[best_idx].ssid, LOG_NET);
        WiFi.begin(appConfig.networks[best_idx].ssid.c_str(), appConfig.networks[best_idx].pass.c_str());

        uint8_t attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20)
        {
            vTaskDelay(pdMS_TO_TICKS(500));
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
    // 1. Load persistent configuration from NVS (with config.h defaults)
    appConfig.load();

    // 2. Initialize presence radar & Ridy-10-Lite HTTP Link client
    radar.init(appConfig.radarEnabled, appConfig.ecoWifiOff, appConfig.screenDelaySec);
    lampClient.autoPresence = appConfig.lampAutoPresence;
    lampClient.begin(appConfig.lampUrl, appConfig.lampKey, appConfig.lampDelaySec);

    String upper_city = appConfig.city;
    upper_city.toUpperCase();
    ui.setOutdoorData(18.0f, 75.0f, 0.0f, "CLOUDY", upper_city.c_str());

    // 3. Connect to strongest saved Wi-Fi or fall back to SoftAP
    if (connectToBestAvailableWiFi())
    {
        _is_connected = true;
        sys_log.log("IP: " + WiFi.localIP().toString(), LOG_NET);

        if (MDNS.begin("climate-station"))
        {
            MDNS.addService("http", "tcp", 80);
            sys_log.log("mDNS: climate-station.local", LOG_NET);
        }

        setupOTA();

        sntp_set_sync_interval(300000);
        configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org", "time.nist.gov");
        sys_log.log("NTP time sync started", LOG_INFO);

        weatherClient.fetchNow();
    }
    else
    {
        _is_connected = false;
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ClimateStation-AP");
        setupOTA();
        sys_log.log("AP Started: 192.168.4.1", LOG_ERR);
    }

    // 4. Start embedded Web Admin server
    webAdmin.begin();

    // 5. Pin unified background network worker to Core 0 (6 KB stack)
    if (_worker_task == nullptr)
    {
        xTaskCreatePinnedToCore(
            workerTaskTrampoline,
            "NetWorkerTask",
            6144,
            this,
            1,
            &_worker_task,
            0 // Core 0
        );
        sys_log.log("NetWorkerTask pinned to Core 0", LOG_INFO);
    }
}

void NetManager::setEcoMode(bool enable_eco, bool deep_wifi_off)
{
    // Non-blocking: queue transition for Core 0 worker so Core 1 UI never stalls
    portENTER_CRITICAL(&_mux);
    _pending_eco_enable = enable_eco;
    _pending_deep_off = deep_wifi_off;
    _eco_dirty = true;
    portEXIT_CRITICAL(&_mux);
}

void NetManager::applyEcoModeInternal(bool enable_eco, bool deep_wifi_off)
{
    if (enable_eco)
    {
        if (deep_wifi_off)
        {
            if (!_wifi_suspended && _is_connected)
            {
                _wifi_suspended = true;
                _is_connected = false;
                WiFi.disconnect(true);
                WiFi.mode(WIFI_OFF);
            }
        }
        else if (_is_connected)
        {
            WiFi.setSleep(true);
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
                if (MDNS.begin("climate-station"))
                {
                    MDNS.addService("http", "tcp", 80);
                }
                sys_log.log("Wi-Fi restored: " + WiFi.localIP().toString(), LOG_NET);
            }
        }
        else if (_is_connected)
        {
            WiFi.setSleep(false);
        }
    }
}

void NetManager::workerTaskTrampoline(void *pvParameters)
{
    NetManager *instance = static_cast<NetManager *>(pvParameters);
    instance->workerLoop();
}

void NetManager::workerLoop()
{
    for (;;)
    {
        // 1. Check for pending Eco-Mode Wi-Fi state changes from Core 1
        bool run_eco = false;
        bool eco_en = false;
        bool deep_off = false;

        portENTER_CRITICAL(&_mux);
        if (_eco_dirty)
        {
            run_eco = true;
            eco_en = _pending_eco_enable;
            deep_off = _pending_deep_off;
            _eco_dirty = false;
        }
        portEXIT_CRITICAL(&_mux);

        if (run_eco)
        {
            applyEcoModeInternal(eco_en, deep_off);
        }

        // 2. Keep connection flag accurate
        if (!_wifi_suspended && WiFi.getMode() == WIFI_STA)
        {
            _is_connected = (WiFi.status() == WL_CONNECTED);
        }

        // 3. Handle OTA, Weather HTTP & Lamp polling exclusively on Core 0
        if (!_wifi_suspended)
        {
            ArduinoOTA.handle();
        }
        weatherClient.process(_wifi_suspended);
        lampClient.pollStatus();

        // Yield 10 ms to Core 0 IDLE / Wi-Fi system tasks
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}