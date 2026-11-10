#include <Arduino.h>
#include "../ui_mod.h"

static lv_obj_t *loc_label = NULL;
static lv_obj_t *main_temp_label = NULL;
static lv_obj_t *status_label = NULL;
static lv_obj_t *line_top = NULL;
static lv_obj_t *line_bottom = NULL;

// Точки для малювання ліній-розділювачів
static lv_point_precise_t line_pts_top[2] = { {30, 100}, {330, 100} };
static lv_point_precise_t line_pts_bottom[2] = { {30, 260}, {330, 260} };

void create_screen_minimal_weather() {
  // Реєструємо екран під індексом 4 (п'ятий по порядку)
  screens[4] = lv_obj_create(NULL);
  
  // ФІКС ЯСКРАВОСТІ ТЛА (Глибокий соковитий чорний)
  lv_obj_set_style_bg_color(screens[4], lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screens[4], LV_OPA_COVER, 0);
  lv_obj_set_style_opa(screens[4], LV_OPA_COVER, 0);

  // 1. Верхній напис - Локація (в стилі Nissan 350z Gauge)
  loc_label = lv_label_create(screens[4]);
  lv_label_set_text(loc_label, "GARAGE TINKERING");
  lv_obj_set_style_text_color(loc_label, lv_color_make(0, 190, 255), 0); // Неоновий блакитний акцент
  lv_obj_set_style_text_font(loc_label, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_opa(loc_label, LV_OPA_COVER, 0);
  lv_obj_align(loc_label, LV_ALIGN_TOP_MID, 0, 55);

  // 2. Верхня лінія-розділювач
  line_top = lv_line_create(screens[4]);
  lv_line_set_points(line_top, line_pts_top, 2);
  lv_obj_set_style_line_color(line_top, lv_color_make(60, 70, 90), LV_PART_MAIN);
  lv_obj_set_style_line_width(line_top, 2, LV_PART_MAIN);

  // 3. Центральний величезний показник температури
  main_temp_label = lv_label_create(screens[4]);
  lv_label_set_text(main_temp_label, "--.-°C");
  lv_obj_set_style_text_color(main_temp_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(main_temp_label, &lv_font_montserrat_32, 0); // Найбільший шрифт проєкту
  lv_obj_set_style_text_opa(main_temp_label, LV_OPA_COVER, 0);
  lv_obj_align(main_temp_label, LV_ALIGN_CENTER, 0, 0);

  // 4. Нижня лінія-розділювач
  line_bottom = lv_line_create(screens[4]);
  lv_line_set_points(line_bottom, line_pts_bottom, 2);
  lv_obj_set_style_line_color(line_bottom, lv_color_make(60, 70, 90), LV_PART_MAIN);
  lv_obj_set_style_line_width(line_bottom, 2, LV_PART_MAIN);

  // 5. Нижня панель статусу - Вологість та Стан
  status_label = lv_label_create(screens[4]);
  lv_label_set_text(status_label, "SYSTEM READY");
  lv_obj_set_style_text_color(status_label, lv_color_make(180, 190, 200), 0);
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_opa(status_label, LV_OPA_COVER, 0);
  lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -65);
}

void update_scr_minimal_weather(float temp, float hum) {
  if (main_temp_label == NULL || status_label == NULL) return;

  // Виводимо температуру
  String temp_str = String(temp, 1) + " °C";
  lv_label_set_text(main_temp_label, temp_str.c_str());

  // Виводимо текстовий статус на основі клімату в гаражі/машині
  String status_str;
  if (temp > 30) {
    status_str = "ENGINE WARM | H: " + String(hum, 0) + "%";
  } else if (temp < 15) {
    status_str = "COLD ENV | H: " + String(hum, 0) + "%";
  } else {
    status_str = "OK STATUS | H: " + String(hum, 0) + "%";
  }
  lv_label_set_text(status_label, status_str.c_str());
}