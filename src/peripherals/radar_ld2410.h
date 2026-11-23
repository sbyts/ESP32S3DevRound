#ifndef RADAR_LD2410_H
#define RADAR_LD2410_H

#include <Arduino.h>
#include "../config/config.h"

class RadarSensor {
private:
    uint8_t _pin;
    bool _enabled;
    bool _deep_wifi_off;
    bool _is_eco_active;
    unsigned long _last_presence_ms;
    unsigned long _grace_period_ms;

public:
    RadarSensor(uint8_t pin) : _pin(pin), _enabled(false), _deep_wifi_off(false), 
                               _is_eco_active(false), _last_presence_ms(0),
                               _grace_period_ms(DEFAULT_SCREEN_DELAY * 1000UL) {}

    void init(bool enabled, bool deep_wifi_off, uint32_t screenDelaySec = DEFAULT_SCREEN_DELAY);
    void setConfig(bool enabled, bool deep_wifi_off, uint32_t screenDelaySec = DEFAULT_SCREEN_DELAY);
    void update();
    void manualWake();

    bool isEnabled() const { return _enabled; }
    bool isDeepWifiOff() const { return _deep_wifi_off; }
    bool isEcoActive() const { return _is_eco_active; }
    uint32_t getGracePeriodSec() const { return _grace_period_ms / 1000UL; }
};

extern RadarSensor radar;

#endif // RADAR_LD2410_H