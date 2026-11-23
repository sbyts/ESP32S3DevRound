# 🗺️ Climate Core OS (ESP32-S3) — Master Architecture & Upgrade Roadmap

## 📊 Current System Baseline (Verified)
* **MCU:** ESP32-S3 (Xtensa LX7 Dual-Core @ 240 MHz, No PSRAM — 338 KB total SRAM)
* **Memory Budget:** 247 KB used / 91 KB free (`Min Free Heap: 58 KB` — ~33 KB safe headroom)
* **Display:** 360×360 Round IPS LCD (GC9B72 SPI, LovyanGFX 8-bit `RGB332` sprite @ 60 FPS)
* **Network & OTA:** Multi-AP Wi-Fi (6 slots), `ESPAsyncWebServer`, mDNS (`climate-station.local`), wireless `ArduinoOTA` (`env:ota`)
* **Current Peripherals:** DHT11 (to be replaced), HLK-LD2410 presence radar (`GPIO 6`), Tactile button (`GPIO 18`)

---

## 🎯 Target Upgrade Objectives
1. **Zero UI Stutter (Dual-Core FreeRTOS):** Offload all blocking network operations to **Core 0**, keeping **Core 1** dedicated to 60 FPS display rendering and radar polling.
2. **High-Precision I2C Sensors:** Replace slow/inaccurate DHT11 with **SHT31** (temp/humidity) and **BH1750** (ambient light in lux) on a shared I2C bus.
3. **Low-Latency ESP-NOW Lamp Link:** Replace HTTP REST polling/control for the **Ridy-10-Lite** desk lamp with direct binary **ESP-NOW** packets (<10 ms latency, router-independent, zero HTTP heap overhead).

---

## 🛠️ Step-by-Step Implementation Plan

### Phase 1: Dual-Core FreeRTOS Foundation (Core 0 / Core 1 Split)
*Goal: Eliminate micro-stutters caused by HTTP weather timeouts and Wi-Fi reconnects.*
- [ ] **Step 1.1 — Thread-Safe Ring Logger (`sys_logger.h/cpp`):**
  * Add `portMUX_TYPE` spinlock critical sections so Core 0 and Core 1 can safely write logs concurrently.
- [ ] **Step 1.2 — Cross-Core UI Data Guard (`ui_manager.h/cpp`):**
  * Ensure `ui.setOutdoorData()` called from Core 0 only updates thread-safe state variables, while SPI display flushing runs strictly on Core 1.
- [ ] **Step 1.3 — Background Network Worker (`net_manager.h/cpp`):**
  * Pin a dedicated `NetWorkerTask` (8 KB stack) to **Core 0** handling `ArduinoOTA.handle()`, `weatherClient.process()`, and deferred `setEcoMode()` Wi-Fi transitions.

### Phase 2: Hardware Sensor Upgrade (DHT11 → SHT31 + BH1750)
*Goal: Non-blocking 1–2 ms I2C sensor reads, auto-brightness, and daylight-aware lamp triggers.*
- [ ] **Step 2.1 — Hardware & Pinout (`config.h` & `platformio.ini`):**
  * Assign free I2C pins (`SDA` / `SCL`), remove `DHT` library, and add lightweight `Adafruit_SHT31` & `BH1750` (or direct Wire registers to save Flash/RAM).
- [ ] **Step 2.2 — I2C Sensor Driver (`sensor_climate.h/cpp`):**
  * Poll SHT31 (`0x44`) and BH1750 (`0x23`) every 2000 ms on Core 1 without blocking interrupts.
- [ ] **Step 2.3 — Smart Lux Automation:**
  * Implement smooth PWM display backlight dimming based on ambient lux.
  * Add configurable `lux_threshold` so the presence radar only turns on the Ridy-10-Lite lamp when the room is dark.

### Phase 3: Ultra-Fast ESP-NOW Communication (Station ↔ Ridy-10-Lite)
*Goal: Instant presence-triggered lighting without HTTP socket delays or router dependency.*
- [ ] **Step 3.1 — Binary Protocol Definition (`espnow_protocol.h`):**
  * Define a compact 12-byte packed struct (`magic`, `cmd`, `state`, `brightness`, `lux`, `security_token`) shared between both firmwares.
- [ ] **Step 3.2 — Station ESP-NOW Transmitter (`espnow_link.h/cpp`):**
  * Initialize `esp_now_init()` alongside `WIFI_STA` (locked to router Wi-Fi channel).
  * Send commands asynchronously via FreeRTOS queue on Core 0 and receive instant ACK/status callbacks from the lamp.
  * Add target Lamp MAC Address (`cfg_lamp_mac`) & Lux Threshold fields to the Web Admin UI (`web_ui.h`, `web_script.h`, `app_config.h`).
- [ ] **Step 3.3 — Lamp Receiver Integration (`Ridy-10-Lite` firmware):**
  * Add matching `esp_now_register_recv_cb` handler in the lamp's `NetManager` to trigger `LampController` immediately on packet arrival.

---

## 🚀 Where to Start Tomorrow
1. Send **`src/system/sys_logger.h`** and **`src/system/sys_logger.cpp`** to execute **Step 1.1**.
2. Confirm which two free **GPIO pins** on the ESP32-S3 will be wired to **I2C SDA / SCL** for Phase 2.