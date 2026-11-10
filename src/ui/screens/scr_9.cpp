#include <Arduino.h>
#include "../ui_mod.h"
#include <math.h>

static lv_obj_t *radar_line = NULL;
static lv_obj_t *sweep_angle_label = NULL;

// ФІКС: Оголошуємо radar_pts саме як масив із двох точок
static lv_point_precise_t radar_pts[2] = { {180, 180}, {180, 40} }; 
static int radar_angle = 0;
static uint32_t last_radar_tick = 0;

void create_screen_9_compass() {
  // Записуємо об'єкт у 10-й слот масиву (індекс 9)
  screens[9] = lv_obj_create(NULL);
  
  // ФІКС ЯСКРАВОСТІ ТЛА
  lv_obj_set_style_bg_color(screens[9], lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screens[9], LV_OPA_COVER, 0);
  lv_obj_set_style_opa(screens[9], LV_OPA_COVER, 0);

  // Малюємо кола сітки РЛС
  for (int r = 80; r <= 240; r += 80) {
    lv_obj_t *circle = lv_obj_create(screens[9]);
    lv_obj_set_size(circle, r, r);
    lv_obj_center(circle);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(circle, lv_color_make(0, 100, 40), 0);
    lv_obj_set_style_border_width(circle, 1, 0);
  }

  // Головна лінія променя сканування
  radar_line = lv_line_create(screens[9]);
  lv_line_set_points(radar_line, radar_pts, 2); // Тепер передається коректний масив точок
  lv_obj_set_style_line_color(radar_line, lv_color_make(0, 255, 100), LV_PART_MAIN);
  lv_obj_set_style_line_width(radar_line, 3, LV_PART_MAIN);
  lv_obj_set_style_line_rounded(radar_line, true, LV_PART_MAIN);

  // Текстовий індикатор кута розгортки
  sweep_angle_label = lv_label_create(screens[9]);
  lv_label_set_text(sweep_angle_label, "SCANNING.. 000°");
  lv_obj_set_style_text_color(sweep_angle_label, lv_color_make(0, 255, 100), 0);
  lv_obj_set_style_text_font(sweep_angle_label, &lv_font_montserrat_14, 0);
  lv_obj_align(sweep_angle_label, LV_ALIGN_BOTTOM_MID, 0, -45);
}

void update_radar_animation() {
  if (radar_line == NULL || screens[9] != lv_screen_active()) return;

  if (millis() - last_radar_tick >= 25) {
    last_radar_tick = millis();
    radar_angle = (radar_angle + 3) % 360;
    
    float rad = radar_angle * M_PI / 180.0;
    
    // Оновлюємо координати саме другої точки масиву (кінець променя)
    radar_pts[1].x = 180 + (int32_t)(130 * sin(rad));
    radar_pts[1].y = 180 - (int32_t)(130 * cos(rad));
    
    // Перемальовуємо лінію
    lv_line_set_points(radar_line, radar_pts, 2);

    char angle_buf[32];
    snprintf(angle_buf, sizeof(angle_buf), "SCANNING.. %03d°", radar_angle);
    lv_label_set_text(sweep_angle_label, angle_buf);
  }
}