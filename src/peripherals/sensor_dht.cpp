#include "sensor_dht.h"

SensorDHT climate_sensor;

void SensorDHT::init() {
    _dht.begin();
}

bool SensorDHT::process() {
    // Rigid non-blocking timing check to guarantee hardware stability
    if (millis() - _last_read_time > 2500) {
        _last_read_time = millis();
        
        float h = _dht.readHumidity();
        float t = _dht.readTemperature();
        
        // Validation check against faulty hardware pulses
        if (isnan(h) || isnan(t)) {
            return false;
        }
        
        _cached_temp = t;
        _cached_hum = h;
        return true; 
    }
    return false;
}