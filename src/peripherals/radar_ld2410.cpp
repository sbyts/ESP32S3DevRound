#include "radar_ld2410.h"
#include "../config/config.h"
#include "../display/ui_manager.h"
#include "../network/net_manager.h"
#include "../system/sys_logger.h"

RadarSensor radar(PIN_RADAR_OUT);

void RadarSensor::init(bool enabled, bool deep_wifi_off, uint32_t screenDelaySec) {
    pinMode(_pin, INPUT_PULLDOWN);
    _enabled = enabled;
    _deep_wifi_off = deep_wifi_off;
    uint32_t safe_sec = (screenDelaySec < 1) ? 1 : screenDelaySec;
    _grace_period_ms = safe_sec * 1000UL;
    _is_eco_active = false;
    _last_presence_ms = millis();
    sys_log.log(String("Radar GPIO6: ") + (_enabled ? "ENABLED" : "DISABLED") +
                " (" + String(safe_sec) + "s)", LOG_INFO);
}

void RadarSensor::setConfig(bool enabled, bool deep_wifi_off, uint32_t screenDelaySec) {
    _enabled = enabled;
    _deep_wifi_off = deep_wifi_off;
    uint32_t safe_sec = (screenDelaySec < 1) ? 1 : screenDelaySec;
    _grace_period_ms = safe_sec * 1000UL;
    if (!_enabled) {
        manualWake();
    }
}

void RadarSensor::manualWake() {
    _last_presence_ms = millis();
    _is_eco_active = false;
    setCpuFrequencyMhz(240);
    ui.setPowerSave(false);
    net_service.setEcoMode(false, _deep_wifi_off);
}

void RadarSensor::update() {
    if (!_enabled) return;

    if (digitalRead(_pin) == HIGH) {
        _last_presence_ms = millis();
        if (_is_eco_active) {
            _is_eco_active = false;
            setCpuFrequencyMhz(240);
            ui.setPowerSave(false);
            net_service.setEcoMode(false, _deep_wifi_off);
            sys_log.log("RADAR: Presence -> Wake", LOG_NET);
        }
    } else {
        if (!_is_eco_active && (millis() - _last_presence_ms > _grace_period_ms)) {
            _is_eco_active = true;
            sys_log.log(_deep_wifi_off ? "RADAR: Eco (Screen+WiFi OFF)" : "RADAR: Eco (Screen OFF)", LOG_INFO);
            ui.setPowerSave(true);
            net_service.setEcoMode(true, _deep_wifi_off);
            setCpuFrequencyMhz(80);
        }
    }
}