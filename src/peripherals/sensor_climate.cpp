#include "sensor_climate.h"
#include "../config/config.h"
#include "../display/ui_manager.h"
#include "../system/sys_logger.h"
#include <Wire.h>

ClimateLightSensor climateSensor;

uint8_t ClimateLightSensor::probeAddress(uint8_t addr) {
    Wire.beginTransmission(addr);
    return (Wire.endTransmission() == 0) ? addr : 0;
}

uint8_t ClimateLightSensor::crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x31) : (crc << 1);
        }
    }
    return crc;
}

void ClimateLightSensor::init() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
    delay(10);

    // Auto-detect SHT31 (0x44 default, 0x45 alternate)
    _sht_addr = probeAddress(0x44);
    if (_sht_addr == 0) _sht_addr = probeAddress(0x45);

    if (_sht_addr != 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "I2C SHT31 (0x%02X) OK", _sht_addr);
        sys_log.log(buf, LOG_INFO);
    } else {
        sys_log.log("I2C SHT31 not found!", LOG_ERR);
    }

    // Auto-detect BH1750 (0x23 default, 0x5C alternate)
    _bh_addr = probeAddress(0x23);
    if (_bh_addr == 0) _bh_addr = probeAddress(0x5C);

    if (_bh_addr != 0) {
        // Power On BH1750
        Wire.beginTransmission(_bh_addr);
        Wire.write(0x01);
        Wire.endTransmission();

        char buf[32];
        snprintf(buf, sizeof(buf), "I2C BH1750 (0x%02X) OK", _bh_addr);
        sys_log.log(buf, LOG_INFO);
    } else {
        sys_log.log("I2C BH1750 not found!", LOG_ERR);
    }

    ui.setIndoorData(_temp_c, _humidity);

    // Kick off first measurement immediately
    triggerMeasurements();
    _last_cycle_ms = millis();
}

void ClimateLightSensor::triggerMeasurements() {
    if (_sht_addr != 0) {
        // Single-shot measurement, High Repeatability, Clock Stretching Disabled (0x2400)
        Wire.beginTransmission(_sht_addr);
        Wire.write(0x24);
        Wire.write(0x00);
        Wire.endTransmission();
    }

    if (_bh_addr != 0) {
        // One-Time High-Resolution Mode (1 lx resolution, ~120ms conversion, auto power-down)
        Wire.beginTransmission(_bh_addr);
        Wire.write(0x20);
        Wire.endTransmission();
    }

    _measuring = true;
    _meas_start_ms = millis();
}

void ClimateLightSensor::readMeasurements() {
    _measuring = false;

    // 1. Read 6 bytes from SHT31 (Temp MSB, Temp LSB, Temp CRC, Hum MSB, Hum LSB, Hum CRC)
    if (_sht_addr != 0) {
        if (Wire.requestFrom(_sht_addr, (uint8_t)6) == 6) {
            uint8_t raw[6];
            for (uint8_t i = 0; i < 6; i++) raw[i] = Wire.read();

            if (crc8(&raw[0], 2) == raw[2] && crc8(&raw[3], 2) == raw[5]) {
                uint16_t raw_t = ((uint16_t)raw[0] << 8) | raw[1];
                uint16_t raw_h = ((uint16_t)raw[3] << 8) | raw[4];

                _temp_c   = -45.0f + 175.0f * ((float)raw_t / 65535.0f);
                _humidity = 100.0f * ((float)raw_h / 65535.0f);
                if (_humidity < 0.0f)   _humidity = 0.0f;
                if (_humidity > 100.0f) _humidity = 100.0f;

                ui.setIndoorData(_temp_c, _humidity);
            }
        }
    }

    // 2. Read 2 bytes from BH1750 (Lux MSB, Lux LSB)
    if (_bh_addr != 0) {
        if (Wire.requestFrom(_bh_addr, (uint8_t)2) == 2) {
            uint16_t raw_lux = ((uint16_t)Wire.read() << 8) | Wire.read();
            _lux = (float)raw_lux / 1.2f;
        }
    }
}

void ClimateLightSensor::update() {
    unsigned long now = millis();

    if (_measuring) {
        if (now - _meas_start_ms >= CONV_DELAY_MS) {
            readMeasurements();
        }
    } else {
        if (now - _last_cycle_ms >= POLL_INTERVAL_MS) {
            _last_cycle_ms = now;
            triggerMeasurements();
        }
    }
}