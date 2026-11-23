#include <Arduino.h>
#include "config/config.h"
#include "display/display_lgfx.h"
#include "display/ui_manager.h"
#include "network/net_manager.h"
#include "network/lamp_client.h"
#include "peripherals/sensor_climate.h"
#include "peripherals/radar_ld2410.h"
#include "system/sys_logger.h"

// Global LovyanGFX display instance matching display_lgfx.h
LGFX_GC9B72 lcd;

// Global Ridy-10-Lite Link client instance
LampClient lampClient;

// Button debounce variables
static bool last_btn_state = HIGH;
static unsigned long last_debounce_time = 0;
const unsigned long DEBOUNCE_DELAY = 50;

void handlePhysicalButton() {
    bool reading = digitalRead(BUTTON_PIN);

    if (reading != last_btn_state) {
        last_debounce_time = millis();
    }

    if ((millis() - last_debounce_time) > DEBOUNCE_DELAY) {
        static bool stable_btn_state = HIGH;
        if (reading != stable_btn_state) {
            stable_btn_state = reading;

            // Trigger on button press (active LOW with INPUT_PULLUP)
            if (stable_btn_state == LOW) {
                bool was_sleeping = ui.isSleeping() || radar.isEcoActive();

                // Always force wake-up on button press so the screen never stays dark
                radar.manualWake();

                // Switch to the next screen only if the display was already awake
                if (!was_sleeping) {
                    ui.nextScreen();
                }
            }
        }
    }
    last_btn_state = reading;
}

void setup() {
    Serial.begin(115200);
    delay(200);

    sys_log.log("Booting Climate Core OS...", LOG_INFO);

    // 1. Configure tactile navigation & wake button (GPIO 18)
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // 2. Initialize 360x360 round IPS display & 8-bit sprite BEFORE Wi-Fi to avoid heap fragmentation
    ui.init();
    sys_log.log("Display GC9B72 (8-bit) OK", LOG_INFO);

    // 3. Initialize I2C climate & ambient light sensors (SHT31 + BH1750 on GPIO 15/16)
    climateSensor.init();

    // 4. Initialize Wi-Fi, NTP, HTTP Weather, Web Server, mDNS & spawn Core 0 NetWorkerTask
    net_service.init();
}

void loop() {
    // Core 1 Dedicated Loop: Zero blocking network calls!

    // 1. Monitor HLK-LD2410 presence radar & manage Eco Power-Save mode
    radar.update();

    // 2. Synchronize presence state & auto-off timer with Ridy-10-Lite lamp
    lampClient.updatePresence(digitalRead(PIN_RADAR_OUT) == HIGH);

    // 3. Handle tactile navigation / wake button
    handlePhysicalButton();

    // 4. Non-blocking two-phase I2C poll (SHT31 + BH1750)
    climateSensor.update();

    // 5. Render smooth 60 FPS UI animations (automatically skipped when sleeping)
    ui.tickAnimation();
}