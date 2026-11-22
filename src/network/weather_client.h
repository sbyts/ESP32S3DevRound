#ifndef WEATHER_CLIENT_H
#define WEATHER_CLIENT_H

#include <Arduino.h>

class WeatherClient {
public:
    void fetchNow();
    void process(bool wifiSuspended);
    String getStatus() const { return _status; }

private:
    String _status = "INIT...";
    unsigned long _lastUpdateMs = 0;
    const unsigned long WEATHER_INTERVAL = 900000; // 15 minutes
};

extern WeatherClient weatherClient;

#endif // WEATHER_CLIENT_H