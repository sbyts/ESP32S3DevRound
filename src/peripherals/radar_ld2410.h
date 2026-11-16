#ifndef RADAR_LD2410_H
#define RADAR_LD2410_H

#include <Arduino.h>

class RadarSensor {
private:
    uint8_t _pin;
    bool _enabled;
    bool _deep_wifi_off;
    bool _is_eco_active;
    unsigned long _last_presence_ms;
    
    // Extra 5-second software grace period after sensor OUT goes LOW
    const unsigned long GRACE_PERIOD_MS = 5000;

public:
    RadarSensor(uint8_t pin) : _pin(pin), _enabled(false), _deep_wifi_off(false), 
                               _is_eco_active(false), _last_presence_ms(0) {}

    void init(bool enabled, bool deep_wifi_off);
    void setConfig(bool enabled, bool deep_wifi_off);
    void update();
    void manualWake();

    bool isEnabled() const { return _enabled; }
    bool isDeepWifiOff() const { return _deep_wifi_off; }
    bool isEcoActive() const { return _is_eco_active; }
};

extern RadarSensor radar;

#endif // RADAR_LD2410_H