#ifndef UI_MOD_H
#define UI_MOD_H

#include <lvgl.h>
#include <stdint.h>

// ЗБІЛЬШУЄМО ДО 11 ОКРЕМІШНІХ ЕКРАНІВ
#define TOTAL_SCREENS 11

extern uint8_t current_screen_index;
extern lv_obj_t *screens[TOTAL_SCREENS];

void create_ui();
void update_sensor_data(float temp, float hum);
void switch_to_next_screen();

// Конструктори екранів (додано новий посередині)
void create_screen_1_classic();
void create_screen_2_premium();
void create_screen_3_chart();
void create_screen_4_gauge();
void create_screen_minimal_weather(); // <-- НАШ НОВИЙ ЕКРАН GARAGE TINKERING
void create_screen_5_smart_home();
void create_screen_6_audio();
void create_screen_7_clock();
void create_screen_8_system();
void create_screen_9_compass();
void create_screen_10_menu();

// Функції оновлення даних
void update_scr_classic(float temp, float hum);
void update_scr_premium(float temp, float hum);
void update_scr_chart(float temp);
void update_scr_gauge(float temp);
void update_scr_minimal_weather(float temp, float hum); // <-- Функція оновлення для нового екрана
void update_scr_system();
void update_audio_player_animation();
void update_clock_animation();
void update_radar_animation();
#endif // UI_MOD_H