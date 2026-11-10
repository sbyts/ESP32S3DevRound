#include <Arduino.h>
#include "../ui_mod.h"

static lv_obj_t *track_title = NULL;
static lv_obj_t *artist_title = NULL;
static lv_obj_t *progress_slider = NULL;
static lv_obj_t *time_label = NULL;

// Елементи плеєра (імітація кнопок)
static lv_obj_t *play_btn = NULL;
static lv_obj_t *prev_btn = NULL;
static lv_obj_t *next_btn = NULL;

// Змінні для внутрішньої анімації плеєра
static int32_t current_progress = 0;
static uint32_t last_player_tick = 0;
static uint8_t current_track_idx = 0;

// Список треків для демонстрації
const char* tracks[] = {"Blinding Lights", "Cyberpunk 2077", "Nightcall", "Starboy"};
const char* artists[] = {"The Weeknd", "Hyper", "Kavinsky", "The Weeknd"};

void create_screen_6_audio() {
  // Реєструємо екран під індексом 6 (сьомий по порядку в масиві)
  screens[6] = lv_obj_create(NULL);
  
  // ЯРКИЙ ФІКС ТЛА: Глибокий чорний колір
  lv_obj_set_style_bg_color(screens[6], lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screens[6], LV_OPA_COVER, 0);
  lv_obj_set_style_opa(screens[6], LV_OPA_COVER, 0);

  // 1. Обкладинка альбому у вигляді стильного неонового кола по центру
  lv_obj_t *album_art = lv_obj_create(screens[6]);
  lv_obj_set_size(album_art, 130, 130);
  lv_obj_align(album_art, LV_ALIGN_CENTER, 0, -40);
  lv_obj_set_style_radius(album_art, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(album_art, lv_color_make(30, 20, 40), 0);
  lv_obj_set_style_border_color(album_art, lv_color_make(190, 0, 255), 0); // Фіолетовий неоновий обідок
  lv_obj_set_style_border_width(album_art, 3, 0);
  lv_obj_set_style_bg_opa(album_art, LV_OPA_COVER, 0);

  // Знак ноти або іконка медіа всередині кола
  lv_obj_t *note_icon = lv_label_create(album_art);
  lv_label_set_text(note_icon, LV_SYMBOL_AUDIO);
  lv_obj_set_style_text_color(note_icon, lv_color_make(190, 0, 255), 0);
  lv_obj_set_style_text_font(note_icon, &lv_font_montserrat_32, 0);
  lv_obj_center(note_icon);

  // 2. Назва композиції
  track_title = lv_label_create(screens[6]);
  lv_label_set_text(track_title, tracks[current_track_idx]);
  lv_obj_set_style_text_color(track_title, lv_color_white(), 0);
  lv_obj_set_style_text_font(track_title, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_opa(track_title, LV_OPA_COVER, 0);
  lv_obj_align(track_title, LV_ALIGN_CENTER, 0, 45);

  // 3. Виконавець
  artist_title = lv_label_create(screens[6]);
  lv_label_set_text(artist_title, artists[current_track_idx]);
  lv_obj_set_style_text_color(artist_title, lv_color_make(140, 150, 170), 0);
  lv_obj_set_style_text_font(artist_title, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_opa(artist_title, LV_OPA_COVER, 0);
  lv_obj_align(artist_title, LV_ALIGN_CENTER, 0, 68);

  // 4. Повзунок прогресу (Slider)
  progress_slider = lv_slider_create(screens[6]);
  lv_obj_set_size(progress_slider, 220, 8);
  lv_obj_align(progress_slider, LV_ALIGN_CENTER, 0, 105);
  lv_slider_set_range(progress_slider, 0, 100);
  lv_slider_set_value(progress_slider, 0, LV_ANIM_OFF);
  
  // Стилізація лінії повзунка
  lv_obj_set_style_bg_color(progress_slider, lv_color_make(40, 45, 55), LV_PART_MAIN);     // Тло слайдера
  lv_obj_set_style_bg_color(progress_slider, lv_color_make(0, 220, 140), LV_PART_INDICATOR); // Зелена смуга прогресу
  lv_obj_remove_style(progress_slider, NULL, LV_PART_KNOB); // Ховаємо круглу ручку для мінімалізму

  // 5. Текст часу (0:00)
  time_label = lv_label_create(screens[6]);
  lv_label_set_text(time_label, "0:00 / 3:20");
  lv_obj_set_style_text_color(time_label, lv_color_make(110, 120, 130), 0);
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);
  lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 125);

  // 6. Панель нижніх кнопок керування (Назад | Play | Вперед)
  play_btn = lv_label_create(screens[6]);
  lv_label_set_text(play_btn, LV_SYMBOL_PLAY);
  lv_obj_set_style_text_color(play_btn, lv_color_make(0, 220, 140), 0);
  lv_obj_set_style_text_font(play_btn, &lv_font_montserrat_22, 0);
  lv_obj_align(play_btn, LV_ALIGN_BOTTOM_MID, 0, -35);

  prev_btn = lv_label_create(screens[6]);
  lv_label_set_text(prev_btn, LV_SYMBOL_PREV);
  lv_obj_set_style_text_color(prev_btn, lv_color_white(), 0);
  lv_obj_set_style_text_font(prev_btn, &lv_font_montserrat_18, 0);
  lv_obj_align(prev_btn, LV_ALIGN_BOTTOM_MID, -50, -37);

  next_btn = lv_label_create(screens[6]);
  lv_label_set_text(next_btn, LV_SYMBOL_NEXT);
  lv_obj_set_style_text_color(next_btn, lv_color_white(), 0);
  lv_obj_set_style_text_font(next_btn, &lv_font_montserrat_18, 0);
  lv_obj_align(next_btn, LV_ALIGN_BOTTOM_MID, 50, -37);
}

// Нова логіка анімації, що викликається з loop() через ui_mod.cpp
void update_audio_player_animation() {
  if (progress_slider == NULL || time_label == NULL || screens[6] != lv_screen_active()) return;

  // Рухаємо прогрес кожні 500 мс
  if (millis() - last_player_tick > 500) {
    last_player_tick = millis();
    current_progress++;

    if (current_progress > 100) {
      current_progress = 0;
      // Зміна треку по колу при завершенні смуги прогресу
      current_track_idx = (current_track_idx + 1) % 4;
      lv_label_set_text(track_title, tracks[current_track_idx]);
      lv_label_set_text(artist_title, artists[current_track_idx]);
    }

    // Оновлюємо віджет слайдера
    lv_slider_set_value(progress_slider, current_progress, LV_ANIM_ON);

    // Динамічний прорахунок хвилин та секунд для тексту
    int total_seconds = (int)(current_progress * 2.0); // припустимо, трек йде 200 сек
    int mins = total_seconds / 60;
    int secs = total_seconds % 60;
    
    char time_buf[24];
    snprintf(time_buf, sizeof(time_buf), "%d:%02d / 3:20", mins, secs);
    lv_label_set_text(time_label, time_buf);
  }
}