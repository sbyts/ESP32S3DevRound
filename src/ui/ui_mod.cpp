#include "ui_mod.h"
#include "../peripherals/peripherals.h"
#include "../display/display_lgfx.h"
#include <Arduino.h>

uint8_t current_screen_index = 0;
lv_obj_t *screens[TOTAL_SCREENS] = {NULL};

static bool ui_initialized = false;

// Колбек, який викликається через 350 мс після старту гортання (коли анімація завершилась)
static void invalidate_screen_cb(lv_anim_t * anim) {
  // Змушуємо LVGL повністю стерти старий кеш і примусово перемалювати активний екран
  lv_obj_invalidate(lv_screen_active());
  // Повторно апаратно фіксуємо ШІМ підсвічування плати
  lcd.setBrightness(180);
}

void create_ui() {
  ui_initialized = false;

  create_screen_1_classic();
  create_screen_2_premium();
  create_screen_3_chart();
  create_screen_4_gauge();
  create_screen_minimal_weather(); 
  create_screen_5_smart_home();
  create_screen_6_audio();
  create_screen_7_clock();
  create_screen_8_system();
  create_screen_9_compass();
  create_screen_10_menu();

  for (int i = 0; i < TOTAL_SCREENS; i++) {
    if (screens[i] == NULL) {
      screens[i] = lv_obj_create(NULL);
      lv_obj_set_style_bg_color(screens[i], lv_color_black(), 0);
      lv_obj_set_style_bg_opa(screens[i], LV_OPA_COVER, 0); 
      
      lv_obj_t *lbl = lv_label_create(screens[i]);
      lv_label_set_text_fmt(lbl, "SCREEN %d\n[ DESIGN PLACEHOLDER ]", i + 1);
      
      lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
      lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
      lv_obj_set_style_text_opa(lbl, LV_OPA_COVER, 0);
      lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_center(lbl);
    } else {
      lv_obj_set_style_bg_color(screens[i], lv_color_black(), 0);
      lv_obj_set_style_bg_opa(screens[i], LV_OPA_COVER, 0); 
      lv_obj_set_style_opa(screens[i], LV_OPA_COVER, 0);
    }
  }

  // ФІКС КОМПІЛЯЦІЇ: Завантажуємо суворо перший елемент масиву (індекс 0)
  lv_screen_load(screens[0]); 
  lv_obj_invalidate(screens[0]); 
  lcd.setBrightness(180);
  set_led_color(0, 0, 150); 
  
  ui_initialized = true; 
}

void switch_to_next_screen() {
  if (!ui_initialized) return;

  current_screen_index = (current_screen_index + 1) % TOTAL_SCREENS;

  if (screens[current_screen_index] != NULL) {
    lv_obj_set_style_opa(screens[current_screen_index], LV_OPA_COVER, 0);
    
    // Запускаємо стандартну швидку анімацію гортання LVGL 9.2.2
    lv_screen_load_anim(screens[current_screen_index], LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
    
    // Створюємо фоновий таймер-тригер, який вистрілить через 320 мс (відразу після закінчення зсуву)
    // і викличе наш функціональний фікс яскравості та рендерингу
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, screens[current_screen_index]);
    lv_anim_set_duration(&a, 320);
    lv_anim_set_completed_cb(&a, invalidate_screen_cb);
    lv_anim_start(&a);
  }

  lcd.setBrightness(180);

  switch (current_screen_index) {
    case 0: set_led_color(0, 0, 150);   break;
    case 1: set_led_color(150, 100, 0); break;
    case 2: set_led_color(0, 150, 0);   break;
    case 3: set_led_color(150, 0, 0);   break;
    case 4: set_led_color(0, 180, 255); break;
    default: set_led_color(80, 0, 80);  break;
  }
}

void update_sensor_data(float temp, float hum) {
  if (!ui_initialized || isnan(temp) || isnan(hum)) return;

  update_scr_classic(temp, hum);
  update_scr_premium(temp, hum);
  update_scr_chart(temp);
  update_scr_gauge(temp);
  update_scr_minimal_weather(temp, hum); 
  update_scr_system();
  update_audio_player_animation(); 

  // --- ДОДАЄМО СЮДИ ОБРОБКУ ГОДИННИКА ТА РАДАРА ---
  update_clock_animation();
  update_radar_animation();
}