#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "app_config.h"
#include "weather_client.h"

class NetManager {
private:
    bool _is_connected = false;
    bool _wifi_suspended = false;

    bool connectToBestAvailableWiFi();

public:
    NetManager() = default;

    void init();
    void process();
    void setEcoMode(bool enable_eco, bool deep_wifi_off);

    bool isConnected() const { return _is_connected; }
    bool isRadarEnabled() const { return appConfig.radarEnabled; }
    bool isEcoWifiOff() const { return appConfig.ecoWifiOff; }
    String getCity() const { return appConfig.city; }
    String getWeatherStatus() const { return weatherClient.getStatus(); }
};

extern NetManager net_service;

#endif // NET_MANAGER_H