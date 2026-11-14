#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

class NetManager {
private:
    Preferences _prefs;
    AsyncWebServer _server;
    
    String _ssid;
    String _pass;
    String _city;
    String _weather_status;
    
    bool _is_connected;
    unsigned long _last_weather_update;
    const unsigned long WEATHER_INTERVAL = 900000; // 15 minutes

    void setupWebServer();
    void fetchSimpleWeather();

public:
    NetManager() : _server(80), _weather_status("INIT..."), 
                   _is_connected(false), _last_weather_update(0) {}

    void init();
    void process();
    
    bool isConnected() const { return _is_connected; }
    String getCity() const { return _city; }
    String getWeatherStatus() const { return _weather_status; }
};

extern NetManager net_service;

#endif // NET_MANAGER_H