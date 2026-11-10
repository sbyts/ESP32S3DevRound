#include <Arduino.h>
#include "../ui_mod.h"

static lv_obj_t *clock_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *second_arc = NULL;

static int hours = 10;
static int minutes = 42;
static int seconds = 0;
static uint32_t last_clock_tick = 0;

void create_screen_7_clock() {
  // Записуємо об'єкт у 8-й слот масиву (індекс 7)
  screens[7] = lv_obj_create(NULL);
  
  lv_obj_set_style_bg_color(screens[7], lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screens[7], LV_OPA_COVER, 0);
  lv_obj_set_style_opa(screens[7], LV_OPA_COVER, 0);

  second_arc = lv_arc_create(screens[7]);
  lv_obj_set_size(second_arc, 300, 300);
  lv_obj_center(second_arc);
  lv_arc_set_rotation(second_arc, 270); 
  lv_arc_set_bg_angles(second_arc, 0, 360);
  lv_arc_set_range(second_arc, 0, 60);
  lv_arc_set_value(second_arc, 0);
  lv_obj_remove_style(second_arc, NULL, LV_PART_KNOB); 

  lv_obj_set_style_arc_color(second_arc, lv_color_make(35, 40, 50), LV_PART_MAIN);     
  lv_obj_set_style_arc_width(second_arc, 4, LV_PART_MAIN);
  lv_obj_set_style_arc_color(second_arc, lv_color_make(255, 0, 110), LV_PART_INDICATOR); 
  lv_obj_set_style_arc_width(second_arc, 6, LV_PART_INDICATOR);

  clock_label = lv_label_create(screens[7]);
  lv_label_set_text(clock_label, "10:42");
  lv_obj_set_style_text_color(clock_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(clock_label, &lv_font_montserrat_32, 0); 
  lv_obj_set_style_text_opa(clock_label, LV_OPA_COVER, 0);
  lv_obj_center(clock_label);

  date_label = lv_label_create(screens[7]);
  lv_label_set_text(date_label, "TUE / 10 NOV");
  lv_obj_set_style_text_color(date_label, lv_color_make(140, 155, 175), 0);
  lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);
  lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 40);
}

void update_clock_animation() {
  if (clock_label == NULL || second_arc == NULL || screens[7] != lv_screen_active()) return;

  if (millis() - last_clock_tick >= 1000) {
    last_clock_tick = millis();
    seconds++;

    if (seconds >= 60) {
      seconds = 0;
      minutes++;
      if (minutes >= 60) {
        minutes = 0;
        hours = (hours + 1) % 24;
      }
    }

    lv_arc_set_value(second_arc, seconds);

    // ФІКС: Виділяємо масив символів фіксованого розміру для рядка часу
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", hours, minutes);
    lv_label_set_text(clock_label, time_str);
  }
}