#include "peripherals.h"
#include "../config/config.h"   
#include <DHT.h>

static DHT dht(DHTPIN, DHTTYPE);

// Оголошуємо номери віртуальних каналів ШІМ всередині файлу (0, 1, 2)
#define LED_R_CHANNEL 0
#define LED_G_CHANNEL 1
#define LED_B_CHANNEL 2

void init_peripherals() {
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  // КРИТИЧНИЙ ФІКС: Обов'язково переводимо ніжки в режим ВИХОДУ,
  // щоб відкрити транзисторні ключі всередині ESP32-S3
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);

  // СУМІСНИЙ СИНТАКСИС ШІМ ДЛЯ ЯДРА ESP32 2.X
  // Налаштовуємо канали: ledcSetup(канал, частота, розрядність)
  ledcSetup(LED_R_CHANNEL, LEDC_BASE_FREQ, LEDC_TIMER_RES);
  ledcSetup(LED_G_CHANNEL, LEDC_BASE_FREQ, LEDC_TIMER_RES);
  ledcSetup(LED_B_CHANNEL, LEDC_BASE_FREQ, LEDC_TIMER_RES);

  // Прив'язуємо фізичні піни до налаштованих каналів
  ledcAttachPin(LED_R_PIN, LED_R_CHANNEL);
  ledcAttachPin(LED_G_PIN, LED_G_CHANNEL);
  ledcAttachPin(LED_B_PIN, LED_B_CHANNEL);

  dht.begin();
}

void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
  // Керуємо яскравістю через номери каналів (0, 1, 2)
  ledcWrite(LED_R_CHANNEL, r);
  ledcWrite(LED_G_CHANNEL, g);
  ledcWrite(LED_B_CHANNEL, b);
}

// void check_button() {
//   static bool last_btn_state = HIGH;
//   bool current_state = digitalRead(BUTTON_PIN);
  
//   if (current_state == LOW && last_btn_state == HIGH) {
//     delay(50); // Антибрязкіт
//     if (digitalRead(BUTTON_PIN) == LOW) {
//       switch_to_next_screen(); 
//     }
//   }
//   last_btn_state = current_state;
// }

// void read_dht_sensor() {
//   static uint32_t last_dht_time = 0;
//   if (millis() - last_dht_time > 2500) {
//     last_dht_time = millis();
//     float humidity = dht.readHumidity();
//     float temperature = dht.readTemperature();
//     update_sensor_data(temperature, humidity);
//   }
// }

