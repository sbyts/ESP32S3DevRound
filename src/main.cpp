#include <Arduino.h>
#include <lvgl.h>

// Підключаємо модулі з нашої структури папок
#include "config/config.h"
#include "display/display_lgfx.h"
#include "ui/ui_mod.h"
#include "peripherals/peripherals.h"

// Фізично виділяємо глобальне місце в пам'яті для об'єкта дисплея (Фікс лінкера)
LGFX_GC9B72 lcd; 

// Локальний вказівник на буфер відмальовування
static uint16_t* lv_buf = NULL;

// Функція зворотного виклику для відправки пікселів на екран (Flush Callback)
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  
  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.writePixels((uint16_t *)px_map, w * h);
  lcd.endWrite();
  
  lv_display_flush_ready(disp);
}

void setup() {
  // Запуск логування портів
  Serial.begin(115200);
  
  // 1. Ініціалізація фізичних датчиків, кнопок та каналів LEDC
  init_peripherals();
  
  // 2. Первинний запуск та конфігурація дисплея через LovyanGFX
  lcd.init();
  lcd.setRotation(0);
  
  // ФІКС ЯСКРАВОСТІ №1: Встановлюємо стабільний рівень ШІМ-підсвічування ДО
  // виділення великих блоків оперативної пам'яті
  lcd.setBrightness(180); 
  
  // 3. Безпечне виділення пам'яті під буфер суворо у внутрішній SRAM (без PSRAM)
  // 360 пікселів ширини * 30 рядків висоти * 2 байти (RGB565) = ~21.6 KB
  size_t buf_size = SCREEN_WIDTH * 30 * sizeof(uint16_t);
  lv_buf = (uint16_t*)malloc(buf_size);
  
  if (lv_buf == NULL) {
    Serial.println("❌ Помилка: Не вдалося виділити внутрішню пам'ять для LVGL буфера!");
    while(1) delay(1000); // Зупиняємо виконання для запобігання крашу процесора
  }
  
  // 4. Ініціалізація графічного ядра бібліотеки LVGL 9
  lv_init();
  lv_display_t *disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_buffers(disp, (uint8_t *)lv_buf, NULL, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, my_disp_flush);
  
  // 5. Виклик менеджера створення UI (наші 10 екранів із захистом від NULL покажчиків)
  create_ui();

  // ФІКС ЯСКРАВОСТІ №2: Змушуємо LVGL миттєво відрендерити перший кадр Classic-екрана,
  // щоб вирівняти логіку шарів і закріпити апаратний рівень підсвічування на 180
  lv_timer_handler(); 
  lcd.setBrightness(180); 
  
  Serial.println("🚀 Система успішно запущена в SRAM режимі на 60 FPS!");
}

void loop() {
  static uint32_t lv_last_tick = 0;
  uint32_t current_time = millis();
  
  // Точний розрахунок внутрішнього часу для плавних анімацій LVGL
  lv_tick_inc(current_time - lv_last_tick);
  lv_last_tick = current_time;
  
  // Головний диспетчер задач графічного рушія
  lv_timer_handler();
  
  // Опитування фізичної кнопки (перемикання екранів)
  check_button();
  
  // Зчитування показників DHT11 та розрахунок комфорту / точки роси
  read_dht_sensor();
  
  // Мікропауза для стабілізації планувальника задач FreeRTOS
  delay(4);
}