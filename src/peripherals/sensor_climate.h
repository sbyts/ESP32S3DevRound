#ifndef SENSOR_CLIMATE_H
#define SENSOR_CLIMATE_H

#include <Arduino.h>

class ClimateLightSensor {
private:
    uint8_t _sht_addr = 0;
    uint8_t _bh_addr  = 0;

    bool _measuring = false;
    unsigned long _last_cycle_ms = 0;
    unsigned long _meas_start_ms = 0;

    static const unsigned long POLL_INTERVAL_MS = 2000;
    static const unsigned long CONV_DELAY_MS    = 150;

    float _temp_c   = 22.0f;
    float _humidity = 45.0f;
    float _lux      = 150.0f;

    uint8_t probeAddress(uint8_t addr);
    static uint8_t crc8(const uint8_t* data, size_t len);
    void triggerMeasurements();
    void readMeasurements();

public:
    void init();
    void update();

    bool isShtReady() const { return _sht_addr != 0; }
    bool isBhReady()  const { return _bh_addr != 0; }

    float getTemperature() const { return _temp_c; }
    float getHumidity()    const { return _humidity; }
    float getLux()         const { return _lux; }
};

extern ClimateLightSensor climateSensor;

#endif // SENSOR_CLIMATE_H