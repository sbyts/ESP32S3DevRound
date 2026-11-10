#include <Arduino.h>
#include "../ui_mod.h"

// Зробіть локальні віджети статичними, щоб вони не перетиналися з іншими файлами
static lv_obj_t *temp_arc = NULL;
static lv_obj_t *temp_label = NULL;
static lv_obj_t *hum_label = NULL;

void create_screen_1_classic() {
  screens[0] = lv_obj_create(NULL); // Переконайтеся, що запис йде в screens[0]!
  lv_obj_set_style_bg_color(screens[0], lv_color_black(), 0);
  
  temp_arc = lv_arc_create(screens[0]);
  lv_obj_set_size(temp_arc, 280, 280); 
  lv_obj_center(temp_arc);
  // ... ваш код стилізації дуги ...

  temp_label = lv_label_create(screens[0]);
  lv_obj_center(temp_label);
  lv_obj_set_style_text_color(temp_label, lv_color_white(), 0); 
  lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_32, 0);

  hum_label = lv_label_create(screens[0]);
  lv_obj_align(hum_label, LV_ALIGN_CENTER, 0, 60); 
  lv_obj_set_style_text_color(hum_label, lv_color_white(), 0); 
  lv_obj_set_style_text_font(hum_label, &lv_font_montserrat_18, 0);
}

void update_scr_classic(float temp, float hum) {
  // КРИТИЧНИЙ ЗАХИСТ: якщо віджети ще не створені, виходимо
  if (temp_arc == NULL || temp_label == NULL || hum_label == NULL) return;

  int32_t arc_val = (int32_t)temp;
  if(arc_val < 0) arc_val = 0;
  if(arc_val > 50) arc_val = 50;
  lv_arc_set_value(temp_arc, arc_val);
  
  String t_string = String(temp, 1) + " C";
  String h_string = "Hum: " + String(hum, 0) + "%";
  lv_label_set_text(temp_label, t_string.c_str());
  lv_label_set_text(hum_label, h_string.c_str());
}