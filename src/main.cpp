#include <Arduino.h>
#include "config/config.h"
#include "display/display_lgfx.h"
#include "display/ui_manager.h"
#include "peripherals/sensor_dht.h"

// Instantiate global hardware driver object
LGFX_GC9B72 lcd;

void check_button() {
    static bool last_btn_state = HIGH;
    bool current_state = digitalRead(BUTTON_PIN);
    
    if (current_state == LOW && last_btn_state == HIGH) {
        delay(40); // Debounce delay
        if (digitalRead(BUTTON_PIN) == LOW) {
            ui.nextScreen();
        }
    }
    last_btn_state = current_state;
}

void setup() {
    Serial.begin(115200);
    delay(150);

    // Pin setups using centralized config.h
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Initializing isolated subsystems
    lcd.init();
    lcd.setRotation(0);
    lcd.setBrightness(180);
    lcd.clear(TFT_BLACK);

    ui.init();
    climate_sensor.init();

    // Set initial baseline target (triggers smooth startup animation sweep from 0 to 23)
    ui.setTargetData(23.0f, 45.0f, true);
}

void loop() {
    check_button();

    // Tick smooth 60 FPS gauge animation
    ui.tickAnimation();

    // Asynchronously tick the sensor logic
    if (climate_sensor.process()) {
        ui.setTargetData(climate_sensor.getTemperature(), climate_sensor.getHumidity());
    }

    delay(1); // Watchdog safety delay
}