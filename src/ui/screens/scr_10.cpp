#include <Arduino.h>
#include "../ui_mod.h"

// НАЗВА ФУНКЦІЇ МАЄ БУТИ САМЕ ТАКОЮ:
void create_screen_10_menu() {
  // Записуємо об'єкт строго в 11-й слот масиву (індекс 10)
  screens[10] = lv_obj_create(NULL);
  
  // ФІКС ЯСКРАВОСТІ ТЛА
  lv_obj_set_style_bg_color(screens[10], lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screens[10], LV_OPA_COVER, 0);
  lv_obj_set_style_opa(screens[10], LV_OPA_COVER, 0);

  // Створюємо красиву неонову заглушку майбутнього головного меню
  lv_obj_t *lbl = lv_label_create(screens[10]);
  lv_label_set_text(lbl, "SCREEN 11\n[ MAIN MENU GRID ]\n" LV_SYMBOL_SETTINGS " " LV_SYMBOL_IMAGE " " LV_SYMBOL_AUDIO);
  
  lv_obj_set_style_text_color(lbl, lv_color_make(255, 200, 0), 0); // Золотистий колір інтерфейсу меню
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_opa(lbl, LV_OPA_COVER, 0);
  lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(lbl);
}