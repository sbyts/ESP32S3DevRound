#include "sensor_dht.h"
#include "../config/config.h"
#include "../display/ui_manager.h"
#include "../system/sys_logger.h"
#include <DHT.h>

static DHT dht(DHTPIN, DHTTYPE);
static unsigned long last_dht_read = 0;
static const unsigned long DHT_READ_INTERVAL = 2500;

void init_dht_sensor() {
    dht.begin();
    ui.setIndoorData(22.0f, 45.0f);
}

void update_dht_sensor() {
    if (millis() - last_dht_read < DHT_READ_INTERVAL) {
        return;
    }
    last_dht_read = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        static bool err_logged = false;
        if (!err_logged) {
            sys_log.log("DHT11 read error (GPIO 4)", LOG_ERR);
            err_logged = true;
        }
        return;
    }

    ui.setIndoorData(t, h);
}