# 🗺️ Climate Core OS (ESP32-S3) — Session Handoff & Phase 2 Summary

> **Purpose:** Context synchronization file for continuing development across devices, verifying Phase 2 hardware automation, updating `README.md`, and launching Phase 3 (`ESP-NOW`).

---

## 📊 Current System Baseline (Verified After Phase 2)

* **MCU:** ESP32-S3 (Xtensa LX7 Dual-Core @ 240 MHz, 338 KB internal SRAM, No PSRAM)
* **Memory Telemetry:** `87 KB` Free Heap RAM / `54 KB` Min Free Heap Watermark (`249 / 336 KB` used)
* **Display:** 360×360 Round IPS LCD (`GC9B72` SPI, LovyanGFX 8-bit `RGB332` sprite @ locked **60 FPS**)
* **Peripherals & Pinout (`src/config/config.h`):**
  * **I2C Bus (`400 kHz`):** `SDA = GPIO 15`, `SCL = GPIO 16` (shared 4-wire parallel bus, `3.3V` power)
    * `SHT31` (`0x44`): High-precision temperature & humidity sensor (with hardware CRC-8 check)
    * `BH1750` (`0x23`): Ambient light sensor in lux (One-Time High-Res mode with auto power-down)
  * **Presence Radar:** `HLK-LD2410` digital OUT on `GPIO 6` (`INPUT_PULLDOWN`)
  * **Navigation / Wake Button:** Tactile switch on `GPIO 18` (`INPUT_PULLUP`)
  * **Display Backlight PWM:** `GPIO 14` (`12 kHz`, auto-dimming `35..255`)

---

## ✅ Completed Work Summary

### Phase 1: Asymmetric Dual-Core FreeRTOS Foundation (100% Complete)
1. **Thread-Safe Ring Logger (`src/system/sys_logger.h/cpp`):**
   * Protected concurrent writes from Core 0 and Core 1 using `portMUX_TYPE` spinlocks.
   * Formatted timestamps on stack (`snprintf`) outside critical sections and used `std::move` for zero-heap-fragmentation buffer rotation.
   * Filtered out noisy `"Button press -> Screen ON"` logs.
2. **Cross-Core UI Data Guard (`src/display/ui_manager.h/cpp`):**
   * Eliminated cross-core SPI calls: Core 0 updates fixed-size stack `char` arrays and sets `_outdoor_dirty`, while Core 1 flushes the SPI display strictly within its 60 FPS loop.
3. **Unified Core 0 Network Worker (`src/network/net_manager.h/cpp`, `src/network/lamp_client.h`):**
   * Consolidated `ArduinoOTA`, `WeatherClient`, `LampClient`, and deferred `Eco-Mode` Wi-Fi transitions into a single `NetWorkerTask` (`6144 bytes` stack) pinned to **Core 0**, saving ~6 KB SRAM.
4. **Live Radar Grace Period (`src/peripherals/radar_ld2410.cpp`):**
   * Fixed propagation of configurable `screenDelaySec` into `_grace_period_ms`.

### Phase 2: I2C Sensor Upgrade & Smart Lux Automation (100% Complete)
1. **Dependency Cleanup (`platformio.ini`, `config.h`):**
   * Removed `DHT` & `Adafruit Unified Sensor` libraries; implemented direct register-level `Wire.h` I2C drivers.
2. **Non-Blocking Two-Phase I2C Driver (`src/peripherals/sensor_climate.h/cpp`):**
   * Every `2000 ms`, triggers SHT31 (`0x2400`) and BH1750 (`0x20`) in `<0.2 ms`, then reads results `150 ms` later in `<0.4 ms` with zero frame drops on Core 1.
3. **Smooth PWM Auto-Brightness (`src/display/ui_manager.h/cpp`):**
   * Maps room lux (`0..600+ lx`) to backlight PWM (`35..255`) using 60 FPS exponential smoothing and wakes directly to the ambient-matched brightness without blinding flashes at night.
4. **Daylight-Aware Lamp Trigger & Web UI (`app_config.h/cpp`, `lamp_client.h`, `web_server.cpp`, `web_ui.h`, `web_script.h`):**
   * Added NVS-persisted `luxThreshold` (`DEFAULT_LUX_THRESHOLD = 40 lx`).
   * Added Web Admin telemetry cards (`Room Climate / Light` and `I2C Sensors (SHT / BH)`) and config input (`Ambient Light Threshold for Lamp Trigger`).
5. **Lamp Status Screen Layout (`src/display/screens/scr_lamp.cpp`):**
   * Updated secondary info row to `"<delay> sec | <lux> lux"` with the vertical divider `|` locked at screen center `X = 180`.

---

## 💡 Smart Lamp Automation Logic (`Station ↔ Ridy-10-Lite`)

1. **Presence Detection (`Core 1` — `LampClient::updatePresence`):**
   * **On Arrival (`PIN_RADAR_OUT == HIGH`):**
     * Station display wakes up immediately (to ambient-matched PWM brightness).
     * System checks ambient room light from `BH1750`: `is_dark_enough = !climateSensor.isBhReady() || (climateSensor.getLux() <= appConfig.luxThreshold)`.
     * **If `lux <= luxThreshold` (Dark room) AND lamp is OFF:** Queues an immediate ON command (`_pendingCmd = 1`) for Core 0.
     * **If `lux > luxThreshold` (Daylight):** Lamp is **not** turned on, saving energy while keeping the station display active.
   * **On Departure (`PIN_RADAR_OUT == LOW` for `>400 ms` debounce):**
     * Starts local countdown (`awayElapsedSec`) for smooth UI arc animation.
     * **If lamp is currently ON:** Queues an immediate delayed-OFF command (`_pendingCmd = 0` → `POST /api/link?presence=0&delay=<offDelaySec>`) **regardless of current lux**, ensuring the lamp never stays stuck ON if daylight increases while the user is present.
2. **Command Execution (`Core 0` — `LampClient::process`):**
   * Executes queued HTTP `POST` commands asynchronously on Core 0 without stalling the 60 FPS UI on Core 1.
   * Polls lamp status (`GET /api/link`) every `3 s` when online (`12 s` backoff when offline).

---

## 🧪 Verification Checklist (Run Before Phase 3)

- [ ] **1. Screen 7 (`scr_lamp`) Visual Alignment:** Verify `"36 sec | 36 lux"` displays cleanly with `|` centered at `X = 180` and updates dynamically when covering/illuminating the BH1750 sensor.
- [ ] **2. Auto-Brightness Curve:** Cover BH1750 in a dark room → backlight dims smoothly to `35/255`. Shine a flashlight → backlight ramps smoothly to `255/255`. Wake from sleep in darkness → no 100% brightness flash.
- [ ] **3. Daylight Inhibit Test (`lux > luxThreshold`):** Set threshold to `40 lx` in Web Admin. With room light > `40 lx`, let screen sleep, then trigger radar → screen wakes up, **lamp stays OFF**.
- [ ] **4. Dark Room Trigger Test (`lux <= luxThreshold`):** Darken room (or cover BH1750) and trigger radar → screen wakes up and **lamp turns ON immediately**.
- [ ] **5. Auto-Off Countdown Test:** Leave radar zone while lamp is ON → `"OFF IN Xs"` countdown runs smoothly on Screen 7 and turns lamp OFF after `lampDelaySec`.

---

## 🚀 Next Steps (Tomorrow's Plan)

1. **Documentation Sync:** Update **`README.md`** to document **Phase 2** (I2C SHT31 + BH1750, non-blocking two-phase polling, PWM auto-brightness, daylight-aware automation) and **Phase 3** architecture.
2. **Phase 3: Ultra-Fast ESP-NOW Wireless Link (`Station ↔ Ridy-10-Lite`):**
   * **Step 3.1 — Packed Binary Protocol (`espnow_protocol.h`):** Define a 12-byte packed struct (`magic`, `cmd`, `state`, `brightness`, `lux`, `security_token`) shared by both devices.
   * **Step 3.2 — Station ESP-NOW Transmitter (`espnow_link.h/cpp`):** Initialize `esp_now_init()` alongside `WIFI_STA` (locked to router Wi-Fi channel) on Core 0 for `<10 ms` router-independent triggers + add `cfg_lamp_mac` field to Web Admin UI.
   * **Step 3.3 — Lamp Receiver Integration (`Ridy-10-Lite` firmware):** Add `esp_now_register_recv_cb` handler in the lamp's `NetManager` to control `LampController` instantaneously on packet arrival.