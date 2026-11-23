#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/portmacro.h>
#include "app_config.h"
#include "weather_client.h"

class NetManager {
private:
    volatile bool _is_connected = false;
    volatile bool _wifi_suspended = false;
    bool _ota_initialized = false;

    TaskHandle_t _worker_task = nullptr;
    mutable portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;

    // Deferred Eco-Mode request flags (set from Core 1, executed on Core 0)
    bool _eco_dirty = false;
    bool _pending_eco_enable = false;
    bool _pending_deep_off = false;

    bool connectToBestAvailableWiFi();
    void setupOTA();
    void applyEcoModeInternal(bool enable_eco, bool deep_wifi_off);

    static void workerTaskTrampoline(void* pvParameters);
    void workerLoop();

public:
    NetManager() = default;

    void init();
    void setEcoMode(bool enable_eco, bool deep_wifi_off);

    bool isConnected() const { return _is_connected; }
    bool isRadarEnabled() const { return appConfig.radarEnabled; }
    bool isEcoWifiOff() const { return appConfig.ecoWifiOff; }
    String getCity() const { return appConfig.city; }
    String getWeatherStatus() const { return weatherClient.getStatus(); }
};

extern NetManager net_service;

#endif // NET_MANAGER_H