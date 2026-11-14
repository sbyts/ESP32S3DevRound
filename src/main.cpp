#include <Arduino.h>
#include "config/config.h"
#include "display/display_lgfx.h"
#include "display/ui_manager.h"
#include "peripherals/sensor_dht.h"
#include "network/net_manager.h"

LGFX_GC9B72 lcd;

void check_button() {
    static bool last_btn_state = HIGH;
    static unsigned long last_debounce_time = 0;
    
    bool current_state = digitalRead(BUTTON_PIN);
    
    if (current_state == LOW && last_btn_state == HIGH) {
        if (millis() - last_debounce_time > 180) {
            last_debounce_time = millis();
            ui.nextScreen();
        }
    }
    last_btn_state = current_state;
}

void setup() {
    Serial.begin(115200);
    delay(150);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    lcd.init();
    lcd.setRotation(0);
    lcd.setBrightness(180);
    lcd.clear(TFT_BLACK);

    ui.init();
    climate_sensor.init();

    // 1. Set fallback defaults BEFORE network init
    ui.setIndoorData(23.0f, 45.0f, false);
    ui.setOutdoorData(18.0f, 75.0f, 0.0f, "CLOUDY", "LOADING...");

    // 2. Network init will overwrite defaults with real saved city & live weather
    net_service.init();
}

void loop() {
    check_button();

    // Tick smooth 60 FPS animations for active screen
    ui.tickAnimation();

    // Read room sensor and update outer grapefruit arc
    if (climate_sensor.process()) {
        ui.setIndoorData(climate_sensor.getTemperature(), climate_sensor.getHumidity());
    }

    // Periodic weather updates
    net_service.process();

    delay(1);
}