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

// --- DHT11 SENSOR SETTINGS ---
#define DHTPIN        4
#define DHTTYPE       DHT11    

// --- BUTTON & RGB LED PINS ---
#define BUTTON_PIN    18     // Pushbutton (connected to GND)

#define LED_R_PIN     17     // Red channel pin
#define LED_G_PIN     16     // Green channel pin
#define LED_B_PIN     15     // Blue channel pin

#define LEDC_BASE_FREQ 5000  // LED PWM frequency (5 kHz)
#define LEDC_TIMER_RES 8     // 8-bit resolution (0 - 255)

#endif // CONFIG_H