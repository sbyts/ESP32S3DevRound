#include "../ui_mod.h"
#include <Arduino.h>
static lv_obj_t *main_temp_label_scr2;
static lv_obj_t *card_comfort_val;
static lv_obj_t *card_dew_val;

static lv_obj_t* create_card(lv_obj_t* parent, int32_t w, int32_t h) {
  lv_obj_t* card = lv_obj_create(parent);
  lv_obj_set_size(card, w, h);
  lv_obj_set_style_bg_color(card, lv_color_make(25, 30, 40), 0); 
  lv_obj_set_style_border_color(card, lv_color_make(45, 55, 75), 0); 
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_radius(card, 16, 0); 
  lv_obj_set_style_pad_all(card, 8, 0); 
  return card;
}

void create_screen_2_premium() {
  screens[1] = lv_obj_create(NULL); // Запис строго в screens[1]
  lv_obj_set_style_bg_color(screens[1], lv_color_black(), 0);

  lv_obj_t *title_scr2 = lv_label_create(screens[1]);
  lv_label_set_text(title_scr2, "[ WEATHER STATE ]");
  lv_obj_align(title_scr2, LV_ALIGN_TOP_MID, 0, 45);

  main_temp_label_scr2 = lv_label_create(screens[1]);
  lv_obj_align(main_temp_label_scr2, LV_ALIGN_CENTER, 0, -25);

  lv_obj_t* card_comfort = create_card(screens[1], 120, 85);
  lv_obj_align(card_comfort, LV_ALIGN_BOTTOM_LEFT, 45, -45);
  card_comfort_val = lv_label_create(card_comfort);
  lv_obj_align(card_comfort_val, LV_ALIGN_BOTTOM_MID, 0, -5);

  lv_obj_t* card_dew = create_card(screens[1], 120, 85);
  lv_obj_align(card_dew, LV_ALIGN_BOTTOM_RIGHT, -45, -45);
  card_dew_val = lv_label_create(card_dew);
  lv_obj_align(card_dew_val, LV_ALIGN_BOTTOM_MID, 0, -5);
}

void update_scr_premium(float temp, float hum) {
if (main_temp_label_scr2 == NULL || card_comfort_val == NULL || card_dew_val == NULL) return;

  String main_t_scr2 = String(temp, 1) + " C";
  lv_label_set_text(main_temp_label_scr2, main_t_scr2.c_str());
}
