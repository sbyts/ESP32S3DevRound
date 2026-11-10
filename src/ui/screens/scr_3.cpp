#include "../ui_mod.h"
static lv_obj_t *chart;
static lv_chart_series_t *ser_temp;
void create_screen_3_chart() {
  screens[2] = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screens[2], lv_color_make(10, 15, 24), 0);
  chart = lv_chart_create(screens[2]);
  lv_obj_set_size(chart, 260, 180);
  lv_obj_center(chart);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  ser_temp = lv_chart_add_series(chart, lv_color_make(0, 190, 255), LV_CHART_AXIS_PRIMARY_Y);
}
void update_scr_chart(float temp) {
  if (!chart) return;
  lv_chart_set_next_value(chart, ser_temp, (lv_coord_t)temp);
}
