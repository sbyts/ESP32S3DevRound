#include <Arduino.h>
#include "../ui_mod.h"

static lv_obj_t *scale = NULL;
static lv_obj_t *needle = NULL;
static lv_obj_t *gauge_val_label = NULL;

void create_screen_4_gauge() {
  // 1. Створюємо та ініціалізуємо базовий екран
  screens[3] = lv_obj_create(NULL);
  
  // Примусове 100% чорне тло для підкладки екрана
  lv_obj_set_style_bg_color(screens[3], lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screens[3], LV_OPA_COVER, 0);
  lv_obj_set_style_opa(screens[3], LV_OPA_COVER, 0);

  // 2. Створюємо кругову шкалу
  scale = lv_scale_create(screens[3]);
  lv_obj_set_size(scale, 280, 280);
  lv_obj_center(scale);
  
  // ==========================================================
  // КРИТИЧНИЙ ФІКС ТЬМЯНОСТІ ШКАЛИ ДЛЯ LVGL 9
  // ==========================================================
  // Робимо тло самого віджета шкали повністю прозорим (щоб не було сірого квадрата)
  lv_obj_set_style_bg_opa(scale, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(scale, 0, LV_PART_MAIN);
  
  // Форсуємо 100% яскравість (непрозорість) для всього віджета шкали
  lv_obj_set_style_opa(scale, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_opa(scale, LV_OPA_COVER, LV_PART_ITEMS);
  lv_obj_set_style_opa(scale, LV_OPA_COVER, LV_PART_INDICATOR);
  
  // Яскраво-білий колір та 100% видимість для тексту міток (цифр на шкалі)
  lv_obj_set_style_text_color(scale, lv_color_white(), LV_PART_INDICATOR);
  lv_obj_set_style_text_opa(scale, LV_OPA_COVER, LV_PART_INDICATOR);
  // ==========================================================

  // Налаштування кутів та засічок (270 градусів дуги, старт зі 135)
  lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
  lv_scale_set_range(scale, 0, 50);
  lv_scale_set_angle_range(scale, 270);
  lv_scale_set_rotation(scale, 135);
  lv_scale_set_total_tick_count(scale, 41);
  lv_scale_set_major_tick_every(scale, 8);
  lv_scale_set_label_show(scale, true);

  // Кольори ліній та тіків (робимо їх чіткими та контрастними)
  lv_obj_set_style_line_color(scale, lv_color_make(140, 145, 155), LV_PART_MAIN); // Головна дуга
  lv_obj_set_style_line_color(scale, lv_color_make(180, 185, 195), LV_PART_ITEMS); // Малі засічки
  lv_obj_set_style_line_color(scale, lv_color_white(), LV_PART_INDICATOR);        // Великі засічки

  // 3. Створюємо лінію стрілки
  needle = lv_line_create(scale);
  lv_obj_set_style_line_color(needle, lv_color_make(255, 180, 0), LV_PART_MAIN); // Насичений помаранчевий
  lv_obj_set_style_line_width(needle, 6, LV_PART_MAIN);                         // Товста стрілка
  lv_obj_set_style_line_rounded(needle, true, LV_PART_MAIN);
  lv_obj_set_style_line_opa(needle, LV_OPA_COVER, LV_PART_MAIN);                 // 100% яскравість стрілки

  // 4. Цифровий напис знизу
  gauge_val_label = lv_label_create(screens[3]);
  lv_obj_align(gauge_val_label, LV_ALIGN_BOTTOM_MID, 0, -60);
  lv_obj_set_style_text_font(gauge_val_label, &lv_font_montserrat_22, 0);
  lv_obj_set_style_text_color(gauge_val_label, lv_color_white(), 0);
  lv_obj_set_style_text_opa(gauge_val_label, LV_OPA_COVER, 0); // 100% яскравість цифри
}

void update_scr_gauge(float temp) {
  if (scale == NULL || needle == NULL) return;
  
  // Рухаємо стрілку за правилами LVGL 9 (довжина стрілки 100 пікселів)
  lv_scale_set_line_needle_value(scale, needle, 100, (int32_t)temp);

  // Оновлюємо текст
  String t_str = String(temp, 1) + " C";
  lv_label_set_text(gauge_val_label, t_str.c_str());
}