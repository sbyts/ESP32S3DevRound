#include <lvgl.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <DHT.h>

// --- ПІНИ ДИСПЛЕЯ ---
// SCLK = 12, MOSI = 11, MISO(SDO) = 13, DC = 9, CS = 10, RST = 8, BL = 14, TE = 5

// --- НАЛАШТУВАННЯ ДАТЧИКА DHT11 ---
#define DHTPIN 4          // Датчик DHT11 повернуто на ваш рідний GPIO 4
#define DHTTYPE DHT11    
DHT dht(DHTPIN, DHTTYPE);

// --- ПІНИ ДЛЯ КНОПКИ ТА СВІТЛОДІОДА ---
#define BUTTON_PIN 18     // Кнопка зафіксована на вашому GPIO 18 (другий кінець на GND)

#define LED_R_PIN 17      // Червона ніжка на GPIO 17
#define LED_G_PIN 16      // Зелена ніжка на GPIO 16
#define LED_B_PIN 15      // Синя ніжка на GPIO 15

#define LED_R_CHANNEL 0
#define LED_G_CHANNEL 1
#define LED_B_CHANNEL 2

#define LEDC_BASE_FREQ 5000
#define LEDC_TIMER_RES 8  

// --- ДРАЙВЕР ДИСПЛЕЯ GC9B72 ---
class LGFX_GC9B72 : public lgfx::LGFX_Device {
  lgfx::Panel_GC9B72  _panel_instance; 
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance; 

public:
  LGFX_GC9B72(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;     
      cfg.spi_mode = 0;             
      cfg.freq_write = 40000000;    
      cfg.freq_read  = 16000000;
      cfg.pin_sclk = 12;            
      cfg.pin_mosi = 11;            
      cfg.pin_miso = 13;            
      cfg.pin_dc   = 9;             
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = 10;    
      cfg.pin_rst          = 8;     
      cfg.panel_width      = 360;   
      cfg.panel_height     = 360;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.readable         = true;  
      
      // Істинний фікс чорного тла за вашою підказкою!
      cfg.invert           = false;  
      cfg.rgb_order        = false;  
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 14;              
      cfg.freq   = 12000;           
      cfg.pwm_channel = 1;          
      cfg.invert = false;           
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); 
    }
    setPanel(&_panel_instance); 
  }
};

LGFX_GC9B72 lcd;

#define SCREEN_WIDTH  360
#define SCREEN_HEIGHT 360
static uint16_t* lv_buf;

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.writePixels((uint16_t *)px_map, w * h);
  lcd.endWrite();
  lv_display_flush_ready(disp);
}

// --- ЕКРАНИ ТА ВІДЖЕТИ ---
lv_obj_t *screen1;
lv_obj_t *screen2;
uint8_t current_screen_index = 1;

// Віджети Екрана 1 (Класичний Клімат)
lv_obj_t *temp_arc;
lv_obj_t *temp_label;
lv_obj_t *hum_label;

// Віджети Екрана 2 (Преміум Погода з картками)
lv_obj_t *main_temp_label_scr2;
lv_obj_t *card_comfort_val;
lv_obj_t *card_dew_val;

void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
  ledcWrite(LED_R_CHANNEL, r);
  ledcWrite(LED_G_CHANNEL, g);
  ledcWrite(LED_B_CHANNEL, b);
}

// Розрахунок точки роси за формулою Магнуса
float calculate_dew_point(float temp, float hum) {
  float a = 17.27;
  float b = 237.7;
  float alpha = ((a * temp) / (b + temp)) + log(hum / 100.0);
  return (b * alpha) / (a - alpha);
}

void update_sensor_data(float temp, float hum) {
  if (isnan(temp) || isnan(hum)) return;
  
  // --- ОНОВЛЕННЯ ЕКРАНА 1 ---
  int32_t arc_val = (int32_t)temp;
  if(arc_val < 0) arc_val = 0;
  if(arc_val > 50) arc_val = 50;
  lv_arc_set_value(temp_arc, arc_val);
  
  String t_string = String(temp, 1) + " C";
  String h_string = "Hum: " + String(hum, 0) + "%";
  lv_label_set_text(temp_label, t_string.c_str());
  lv_label_set_text(hum_label, h_string.c_str());
  
  // --- ОНОВЛЕННЯ ЕКРАНА 2 ---
  String main_t_scr2 = String(temp, 1) + " C";
  lv_label_set_text(main_temp_label_scr2, main_t_scr2.c_str());
  
  // Розрахунок та вивід точки роси
  float dew_point = calculate_dew_point(temp, hum);
  String dew_str = String(dew_point, 1) + " C";
  lv_label_set_text(card_dew_val, dew_str.c_str());
  
  // Оцінка індексу комфорту
  if (temp > 28 || hum > 70) {
    lv_label_set_text(card_comfort_val, "Hot/Humid");
  } else if (temp < 18) {
    lv_label_set_text(card_comfort_val, "Cool");
  } else {
    lv_label_set_text(card_comfort_val, "Pleasant");
  }
}

// Функція для створення карток на Екрані 2
lv_obj_t* create_card(lv_obj_t* parent, int32_t w, int32_t h) {
  lv_obj_t* card = lv_obj_create(parent);
  lv_obj_set_size(card, w, h);
  lv_obj_set_style_bg_color(card, lv_color_make(25, 30, 40), 0); 
  lv_obj_set_style_border_color(card, lv_color_make(45, 55, 75), 0); 
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_radius(card, 16, 0); 
  lv_obj_set_style_pad_all(card, 8, 0); 
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE); 
  return card;
}

void create_ui() {
  // ==========================================
  // --- ЕКРАН 1: ОРИГІНАЛЬНИЙ ЧОРНИЙ КЛІМАТ (ЯСКРАВИЙ) ---
  // ==========================================
  screen1 = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen1, lv_color_black(), 0);
  
  temp_arc = lv_arc_create(screen1);
  lv_obj_set_size(temp_arc, 280, 280); 
  lv_obj_center(temp_arc);
  lv_arc_set_rotation(temp_arc, 135);     
  lv_arc_set_bg_angles(temp_arc, 0, 270); 
  lv_arc_set_range(temp_arc, 0, 50); 
  lv_obj_remove_style(temp_arc, NULL, LV_PART_KNOB); 
  
  // ФІКС ТЕМНОТИ: Робимо фонову дугу світло-сірою, щоб вона не зливалася з тлом (було 70, 70, 70)
  lv_obj_set_style_arc_color(temp_arc, lv_color_make(130, 135, 145), LV_PART_MAIN); 
  lv_obj_set_style_arc_width(temp_arc, 12, LV_PART_MAIN);
  
  // Неоновий яскраво-блакитний колір активної шкали температури
  lv_obj_set_style_arc_color(temp_arc, lv_color_make(0, 190, 255), LV_PART_INDICATOR); 
  lv_obj_set_style_arc_width(temp_arc, 14, LV_PART_INDICATOR);

  temp_label = lv_label_create(screen1);
  lv_obj_center(temp_label);
  lv_obj_set_style_text_color(temp_label, lv_color_white(), 0); 
  lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_32, 0);

  hum_label = lv_label_create(screen1);
  lv_obj_align(hum_label, LV_ALIGN_CENTER, 0, 60); 
  
  // ФІКС ТЕМНОТИ: Змінюємо бляклий сірий колір на чистий контрастний білий (було 170, 170, 170)
  lv_obj_set_style_text_color(hum_label, lv_color_white(), 0); 
  lv_obj_set_style_text_font(hum_label, &lv_font_montserrat_18, 0);

  // ==========================================
  // --- ЕКРАН 2: ПРЕМІУМ ДИЗАЙН З КАРТКАМИ ---
  // ==========================================
  screen2 = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen2, lv_color_black(), 0);
  
  // Верхній заголовок екрана 2
  lv_obj_t *title_scr2 = lv_label_create(screen2);
  lv_label_set_text(title_scr2, "[ WEATHER STATE ]");
  lv_obj_set_style_text_color(title_scr2, lv_color_make(140, 155, 180), 0);
  lv_obj_set_style_text_font(title_scr2, &lv_font_montserrat_14, 0);
  lv_obj_align(title_scr2, LV_ALIGN_TOP_MID, 0, 45);

  // Центральний показник температури
  main_temp_label_scr2 = lv_label_create(screen2);
  lv_label_set_text(main_temp_label_scr2, "--.- C");
  lv_obj_set_style_text_color(main_temp_label_scr2, lv_color_white(), 0);
  lv_obj_set_style_text_font(main_temp_label_scr2, &lv_font_montserrat_32, 0); 
  lv_obj_align(main_temp_label_scr2, LV_ALIGN_CENTER, 0, -25);

  // Картка Comfort (Ліворуч)
  lv_obj_t* card_comfort = create_card(screen2, 120, 85);
  lv_obj_align(card_comfort, LV_ALIGN_BOTTOM_LEFT, 45, -45);
  
  lv_obj_t* card_comfort_title = lv_label_create(card_comfort);
  lv_label_set_text(card_comfort_title, LV_SYMBOL_CHARGE " Comfort");
  lv_obj_set_style_text_color(card_comfort_title, lv_color_make(0, 200, 120), 0); 
  lv_obj_set_style_text_font(card_comfort_title, &lv_font_montserrat_14, 0);
  lv_obj_align(card_comfort_title, LV_ALIGN_TOP_MID, 0, 0);
  
  card_comfort_val = lv_label_create(card_comfort);
  lv_label_set_text(card_comfort_val, "---");
  lv_obj_set_style_text_color(card_comfort_val, lv_color_white(), 0);
  lv_obj_set_style_text_font(card_comfort_val, &lv_font_montserrat_14, 0);
  lv_obj_align(card_comfort_val, LV_ALIGN_BOTTOM_MID, 0, -5);

  // Картка Точки Роси / Dew Point (Праворуч)
  lv_obj_t* card_dew = create_card(screen2, 120, 85);
  lv_obj_align(card_dew, LV_ALIGN_BOTTOM_RIGHT, -45, -45);
  
  lv_obj_t* card_dew_title = lv_label_create(card_dew);
  lv_label_set_text(card_dew_title, LV_SYMBOL_SETTINGS " Dew P.");
  lv_obj_set_style_text_color(card_dew_title, lv_color_make(255, 140, 0), 0); 
  lv_obj_set_style_text_font(card_dew_title, &lv_font_montserrat_14, 0);
  lv_obj_align(card_dew_title, LV_ALIGN_TOP_MID, 0, 0);
  
  card_dew_val = lv_label_create(card_dew);
  lv_label_set_text(card_dew_val, "-- C");
  lv_obj_set_style_text_color(card_dew_val, lv_color_white(), 0);
  lv_obj_set_style_text_font(card_dew_val, &lv_font_montserrat_14, 0);
  lv_obj_align(card_dew_val, LV_ALIGN_BOTTOM_MID, 0, -5);

  // Запуск: завантажуємо перший екран та вмикаем синій світлодіод
  lv_screen_load(screen1);
  set_led_color(0, 0, 150); 
}

void check_button() {
  static bool last_btn_state = HIGH;
  bool current_state = digitalRead(BUTTON_PIN);
  
  if (current_state == LOW && last_btn_state == HIGH) {
    delay(50); // Антибрязкіт контакту
    if (digitalRead(BUTTON_PIN) == LOW) {
      if (current_screen_index == 1) {
        lv_screen_load_anim(screen2, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
        current_screen_index = 2;
        set_led_color(150, 120, 0); // Жовтий світлодіод для другого екрана
      } else {
        lv_screen_load_anim(screen1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
        current_screen_index = 1;
        set_led_color(0, 0, 150);  // Синій світлодіод для першого екрана
      }
    }
  }
  last_btn_state = current_state;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  ledcSetup(LED_R_CHANNEL, LEDC_BASE_FREQ, LEDC_TIMER_RES);
  ledcAttachPin(LED_R_PIN, LED_R_CHANNEL);

  ledcSetup(LED_G_CHANNEL, LEDC_BASE_FREQ, LEDC_TIMER_RES);
  ledcAttachPin(LED_G_PIN, LED_G_CHANNEL);

  ledcSetup(LED_B_CHANNEL, LEDC_BASE_FREQ, LEDC_TIMER_RES);

ledcAttachPin(LED_B_PIN, LED_B_CHANNEL);
size_t buf_size = SCREEN_WIDTH * 80 * sizeof(uint16_t);
lv_buf = (uint16_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
if (lv_buf == NULL) {
buf_size = SCREEN_WIDTH * 30 * sizeof(uint16_t);
lv_buf = (uint16_t*)malloc(buf_size);
}
dht.begin();
lcd.init();
lcd.setRotation(0);
lcd.setBrightness(180);
lv_init();
lv_display_t *disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
lv_display_set_buffers(disp, (uint8_t *)lv_buf, NULL, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
lv_display_set_flush_cb(disp, my_disp_flush);
create_ui();
}
void loop() {
static uint32_t lv_last_tick = 0;
uint32_t current_time = millis();
lv_tick_inc(current_time - lv_last_tick);
lv_last_tick = current_time;
lv_timer_handler();
check_button();
delay(4);
static uint32_t last_dht_time = 0;
if (millis() - last_dht_time > 2500) {
last_dht_time = millis();
float humidity = dht.readHumidity();
float temperature = dht.readTemperature();
update_sensor_data(temperature, humidity);
}
}