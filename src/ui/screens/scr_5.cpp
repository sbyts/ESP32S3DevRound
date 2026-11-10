#include <Arduino.h>
#include "../ui_mod.h"

void create_screen_5_smart_home() {
  // Реєструємо екран під індексом 5 (шостий по порядку в масиві)
  screens[5] = lv_obj_create(NULL);
  
  // ФІКС ЯСКРАВОСТІ ТЛА (Чистий глибокий чорний)
  lv_obj_set_style_bg_color(screens[5], lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screens[5], LV_OPA_COVER, 0);
  lv_obj_set_style_opa(screens[5], LV_OPA_COVER, 0);

  // 1. Заголовок екрана
  lv_obj_t *title = lv_label_create(screens[5]);
  lv_label_set_text(title, "SMART HOME CONTROL");
  lv_obj_set_style_text_color(title, lv_color_make(0, 220, 255), 0); // Неоновий блакитний
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_opa(title, LV_OPA_COVER, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 45);

  // 2. Блок керування кліматом (іконка + статус кондиціонера)
  lv_obj_t *ac_icon = lv_label_create(screens[5]);
  lv_label_set_text(ac_icon, LV_SYMBOL_REFRESH " Climate AC");
  lv_obj_set_style_text_color(ac_icon, lv_color_white(), 0);
  lv_obj_set_style_text_font(ac_icon, &lv_font_montserrat_14, 0);
  lv_obj_align(ac_icon, LV_ALIGN_CENTER, -40, -45);

  // Перемикач для кондиціонера (АКТИВНИЙ, увімкнений за замовчуванням)
  lv_obj_t *ac_sw = lv_switch_create(screens[5]);
  lv_obj_set_size(ac_sw, 45, 22);
  lv_obj_align(ac_sw, LV_ALIGN_CENTER, 60, -45);
  lv_obj_add_state(ac_sw, LV_STATE_CHECKED); // Вмикаємо
  lv_obj_set_style_bg_color(ac_sw, lv_color_make(0, 200, 100), LV_PART_INDICATOR); // Зелений колір активного стану

  // 3. Блок керування основним світлом (Living Room Light)
  lv_obj_t *light_icon = lv_label_create(screens[5]);
  lv_label_set_text(light_icon, LV_SYMBOL_EYE_OPEN " Main Light");
  lv_obj_set_style_text_color(light_icon, lv_color_white(), 0);
  lv_obj_set_style_text_font(light_icon, &lv_font_montserrat_14, 0);
  lv_obj_align(light_icon, LV_ALIGN_CENTER, -40, 0);

  lv_obj_t *light_sw = lv_switch_create(screens[5]);
  lv_obj_set_size(light_sw, 45, 22);
  lv_obj_align(light_sw, LV_ALIGN_CENTER, 60, 0);
  lv_obj_set_style_bg_color(light_sw, lv_color_make(0, 200, 100), LV_PART_INDICATOR);

  // 4. Слайдер регулювання яскравості підсвічування (Dimmer)
  lv_obj_t *dimmer_label = lv_label_create(screens[5]);
  lv_label_set_text(dimmer_label, "Brightness: 75%");
  lv_obj_set_style_text_color(dimmer_label, lv_color_make(150, 160, 180), 0);
  lv_obj_set_style_text_font(dimmer_label, &lv_font_montserrat_14, 0);
  lv_obj_align(dimmer_label, LV_ALIGN_CENTER, 0, 45);

  lv_obj_t *brightness_slider = lv_slider_create(screens[5]);
  lv_obj_set_size(brightness_slider, 200, 8);
  lv_obj_align(brightness_slider, LV_ALIGN_CENTER, 0, 75);
  lv_slider_set_value(brightness_slider, 75, LV_ANIM_OFF);
  
  // Стилізація слайдера
  lv_obj_set_style_bg_color(brightness_slider, lv_color_make(40, 45, 55), LV_PART_MAIN); 
  lv_obj_set_style_bg_color(brightness_slider, lv_color_make(255, 180, 0), LV_PART_INDICATOR); // Помаранчевий рівень
  lv_obj_set_style_bg_color(brightness_slider, lv_color_make(255, 200, 0), LV_PART_KNOB);      // Яскрава ручка повзунка
}