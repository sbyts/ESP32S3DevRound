#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- НАЛАШТУВАННЯ ЕКРАНА ---
#define SCREEN_WIDTH  360
#define SCREEN_HEIGHT 360

// --- НАЛАШТУВАННЯ ДАТЧИКА DHT11 ---
#define DHTPIN 4
#define DHTTYPE DHT11    

// --- ПІНИ ДЛЯ КНОПКИ ТА СВІТЛОДІОДА ---
#define BUTTON_PIN 18     // Кнопка (другий кінець на GND)

#define LED_R_PIN 17      // Червона ніжка
#define LED_G_PIN 16      // Зелена ніжка
#define LED_B_PIN 15      // Синя ніжка

#define LEDC_BASE_FREQ 5000  // Частота ШІМ (5 кГц)
#define LEDC_TIMER_RES 8     // Розрядність 8 біт (значення від 0 до 255)

#endif // CONFIG_H