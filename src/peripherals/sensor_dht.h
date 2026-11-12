#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <DHT.h>
#include "../config/config.h"

// Hardware pin fallbacks if not defined in config.h
#ifndef DHTPIN
#define DHTPIN 5       
#endif

#ifndef DHTTYPE
#define DHTTYPE DHT11  
#endif

// Class responsible for asynchronous, non-blocking sensor readings
class SensorDHT {
private:
    DHT _dht;
    unsigned long _last_read_time;
    float _cached_temp;
    float _cached_hum;

public:
    SensorDHT() : _dht(DHTPIN, DHTTYPE), _last_read_time(0), _cached_temp(-999.0f), _cached_hum(-999.0f) {}

    // Initializes the physical DHT communication protocol
    void init();

    // Ticks the internal timer and reads data safely every 2.5 seconds. Returns true on a new valid reading.
    bool process();

    // Data getters
    float getTemperature() const { return _cached_temp; }
    float getHumidity() const { return _cached_hum; }
};

// Global instance declaration to allow access within the UI module
extern SensorDHT climate_sensor;

#endif // SENSOR_DHT_H