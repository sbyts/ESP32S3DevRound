#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

struct WifiCreds {
    String ssid;
    String pass;
};

class NetManager {
public:
    static const uint8_t MAX_WIFI_NETWORKS = 6;

private:
    Preferences _prefs;
    AsyncWebServer _server;
    
    WifiCreds _networks[MAX_WIFI_NETWORKS];
    uint8_t _net_count;
    
    String _city;
    String _admin_pass;
    String _weather_status;
    uint32_t _boot_count;
    
    bool _is_connected;
    unsigned long _last_weather_update;
    const unsigned long WEATHER_INTERVAL = 900000;

    void loadSettings();
    void saveNetworksToPrefs();
    bool connectToBestAvailableWiFi();
    void setupWebServer();
    void fetchSimpleWeather();
    bool checkAuth(AsyncWebServerRequest *request);

public:
    NetManager() : _server(80), _net_count(0), _weather_status("INIT..."), 
                   _boot_count(0), _is_connected(false), _last_weather_update(0) {}

    void init();
    void process();
    
    bool isConnected() const { return _is_connected; }
    String getCity() const { return _city; }
    String getWeatherStatus() const { return _weather_status; }
};

extern NetManager net_service;

#endif // NET_MANAGER_H