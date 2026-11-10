#include "../ui_mod.h"
#include <Arduino.h>
static lv_obj_t *ram_bar;
void create_screen_8_system() {
  screens[7] = lv_obj_create(NULL);
  ram_bar = lv_bar_create(screens[7]);
  lv_obj_set_size(ram_bar, 200, 20);
  lv_obj_center(ram_bar);
}
void update_scr_system() {
  if (!ram_bar) return;
  lv_bar_set_value(ram_bar, 50, LV_ANIM_ON);
}
