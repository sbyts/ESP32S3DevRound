#include <lvgl.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <DHT.h>


// --- НАЛАШТУВАННЯ ДАТЧИКА DHT11 ---
#define DHTPIN 4         // Пін даних DHT11
#define DHTTYPE DHT11    // Тип датчика
DHT dht(DHTPIN, DHTTYPE);


// --- 1. КОНФІГУРАЦІЯ ДИСПЛЕЯ LOVYANGFX (ВИПРАВЛЕНО) ---
class LGFX_GC9B72 : public lgfx::LGFX_Device {
  lgfx::Panel_GC9B72  _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance; // Додаємо модуль керування підсвічуванням через PWM


public:
  LGFX_GC9B72(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;     // Для ESP32-S3 використовується SPI2 (FSPI)
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;    // Швидкість запису 40 MHz
      cfg.freq_read  = 16000000;
      cfg.pin_sclk = 12;            // Ваш SCLK пін
      cfg.pin_mosi = 11;            // Ваш MOSI пін
      cfg.pin_miso = -1;            // Не використовується
      cfg.pin_dc   = 9;             // Ваш DC пін
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = 10;    // Ваш CS пін
      cfg.pin_rst          = 8;     // Ваш RST пін
      cfg.panel_width      = 360;   // Круглий екран 360x360
      cfg.panel_height     = 360;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.readable         = false;
      cfg.invert           = true;  // Інверсія кольорів
      cfg.rgb_order        = false;
      _panel_instance.config(cfg);
    }
    {
      // ПРАВИЛЬНЕ НАЛАШТУВАННЯ ПІДCВІЧУВАННЯ (BL)
      auto cfg = _light_instance.config();
      cfg.pin_bl = 14;              // Ваш BL пін (GPIO 14)
      cfg.freq   = 12000;           // Частота ШІМ для регулювання яскравості
      cfg.pwm_channel = 1;          // Номер ШІМ-каналу ESP32
      cfg.invert = false;           // true, якщо підсвічування активується низьким рівнем (GND)
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // Прив'язуємо підсвічування до панелі
    }
    setPanel(&_panel_instance);
  }
};


LGFX_GC9B72 lcd;


// --- 2. БУФЕРИ ТА ФУНКЦІЯ FLUSH ДЛЯ LVGL 9 ---
#define SCREEN_WIDTH  360
#define SCREEN_HEIGHT 360
// Створюємо буфер на 40 рядків екрана
static uint8_t lv_buf[SCREEN_WIDTH * 40 * sizeof(lv_color_t)];


void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);


  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.writePixels((uint16_t *)px_map, w * h);
  lcd.endWrite();


  lv_display_flush_ready(disp);
}


// Глобальні об'єкти інтерфейсу LVGL
lv_obj_t *temp_arc;
lv_obj_t *temp_label;
lv_obj_t *hum_label;


// --- 3. ФУНКЦІЇ ОНОВЛЕННЯ ДАНИХ ТА СТВОРЕННЯ UI ---


// Функція виведення актуальних даних на екран
void update_temperature_display(float temp, float hum) {
  // Перевірка на помилку зчитування (isnan)
  if (isnan(temp) || isnan(hum)) {
    lv_label_set_text(temp_label, "--.-°C");
    lv_label_set_text(hum_label, "Hum: --%");
    return;
  }


  // Оновлюємо положення дуги (округлюємо до цілого числа)
  lv_arc_set_value(temp_arc, (int32_t)temp);
 
  // Виводимо температуру по центру
  char temp_str[16];
  snprintf(temp_str, sizeof(temp_str), "%.1f °C", temp);
  lv_label_set_text(temp_label, temp_str);


  // Виводимо вологість трохи нижче
  char hum_str[16];
  snprintf(hum_str, sizeof(hum_str), "Hum: %.0f%%", hum);
  lv_label_set_text(hum_label, hum_str);
}


// Ініціалізація віджетів круглого екрана
void create_temperature_ui() {
  // Створюємо фонову дугу (шкалу термометра)
  temp_arc = lv_arc_create(lv_screen_active());
  lv_obj_set_size(temp_arc, 280, 280); // Розмір під екран 360x360
  lv_obj_center(temp_arc);
 
  // Кути дуги: починається знизу ліворуч (135°), закінчується знизу праворуч (45°)
  lv_arc_set_angles(temp_arc, 135, 45);
  lv_arc_set_range(temp_arc, 0, 50); // Діапазон DHT11 (від 0 до 50°C)
  lv_obj_remove_style(temp_arc, NULL, LV_PART_KNOB); // Вимикаємо повзунок (робимо чисто індикатором)


  // Головний текстовий блок для температури (Центр)
  temp_label = lv_label_create(lv_screen_active());
  lv_obj_center(temp_label);
  lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_32, 0);


  // Додатковий текстовий блок для вологості (Нижче центру)
  hum_label = lv_label_create(lv_screen_active());
  lv_obj_align(hum_label, LV_ALIGN_CENTER, 0, 50); // Зміщення на 50 пікселів вниз
  lv_obj_set_style_text_font(hum_label, &lv_font_montserrat_18, 0);
 
  // Початковий стан екрана до першого опитування датчика
  update_temperature_display(NAN, NAN);
}


// --- 4. ОСНОВНИЙ ЦИКЛ ПРОГРАМИ ---


void setup() {
  Serial.begin(115200);


  // Старт датчика DHT11
  dht.begin();


  // Старт та базове налаштування LovyanGFX
  lcd.init();
  lcd.setRotation(0);
  lcd.setBrightness(150); // Яскравість підсвічування (0-255)


  // Ініціалізація ядра LVGL
  lv_init();
 
  // Реєстрація дисплея в LVGL 9
  lv_display_t *disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_buffers(disp, lv_buf, NULL, sizeof(lv_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, my_disp_flush);


  // Створення графіки
  create_temperature_ui();
}


void loop() {
  // Обробник внутрішніх процесів та анімацій LVGL
  lv_tick_inc(5);
  lv_timer_handler();
  delay(5);


  // Періодичне зчитування даних з DHT11 (кожні 2 секунди)
  static uint32_t last_read_time = 0;
  if (millis() - last_read_time > 2000) {
    last_read_time = millis();
   
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
   
    // Оновлюємо екран отриманими даними
    update_temperature_display(temperature, humidity);
  }
}
