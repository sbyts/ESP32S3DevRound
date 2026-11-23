#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- DISPLAY DIMENSIONS ---
#define SCREEN_WIDTH  360
#define SCREEN_HEIGHT 360

// --- DISPLAY SPI & CONTROL PINS (GC9B72) ---
#define LCD_SCLK_PIN  12
#define LCD_MOSI_PIN  11
#define LCD_MISO_PIN  13
#define LCD_DC_PIN    9
#define LCD_CS_PIN    10
#define LCD_RST_PIN   8
#define LCD_BL_PIN    14

// --- DISPLAY BACKLIGHT PWM ---
#define LCD_BL_FREQ   12000  // 12 kHz anti-flicker frequency
#define LCD_BL_CHAN   1      // PWM channel
#define LCD_BL_MIN    35     // Minimum display brightness in darkness (0..255)
#define LCD_BL_MAX    255    // Maximum display brightness in daylight (0..255)

// --- I2C BUS (SHT31 + BH1750) ---
#define I2C_SDA_PIN   15     // Freed from unused RGB LED
#define I2C_SCL_PIN   16     // Freed from unused RGB LED
#define I2C_FREQ_HZ   400000 // 400 kHz Fast-Mode I2C

// --- BUTTON & PRESENCE RADAR ---
#define BUTTON_PIN    18     // Pushbutton (connected to GND, INPUT_PULLUP)
#define PIN_RADAR_OUT 6      // HLK-LD2410 digital presence output

// --- SMART AUTOMATION & LAMP LINK DEFAULTS ---
#define DEFAULT_LAMP_URL      "http://ridy-lamp.local"
#define DEFAULT_LAMP_KEY      "ridy-desk-2026"
#define DEFAULT_LAMP_DELAY    10   // Lamp auto-off delay in seconds
#define DEFAULT_SCREEN_DELAY  15   // Station display sleep delay in seconds
#define DEFAULT_LUX_THRESHOLD 40   // Ambient light threshold (lux) for lamp trigger

#endif // CONFIG_H