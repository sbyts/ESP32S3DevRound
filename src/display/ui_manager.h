#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include "display/display_lgfx.h"
#include "display/display_colors.h"

// Class controlling layout states, smooth animations, and anti-aliased rendering
class UIManager {
private:
    LGFX_Sprite _spr;
    uint8_t _current_screen;
    const uint8_t _total_screens;

    // Animation and telemetry state variables
    float _target_temp;
    float _display_temp;
    float _current_hum;
    String _location_name;
    unsigned long _last_anim_time;

    // Internal low-level primitive vector components
    void draw_weather_vector_icon(int32_t x, int32_t y);
    void draw_calibration_grid(const char* title, const DisplayColor* color_array, uint8_t size);
    
    // Explicit screen layouts
    void draw_screen_premium(float temp, float hum);
    void draw_screen_palette_core();
    void draw_screen_palette_design();

public:
    UIManager() : _spr(&lcd), _current_screen(0), _total_screens(3), 
                  _target_temp(0.0f), _display_temp(0.0f), _current_hum(0.0f), 
                  _location_name("KYIV"), _last_anim_time(0) {}

    void init();
    void nextScreen();
    void setTargetData(float temp, float hum, bool force_redraw = false);
    
    // Sets the city/location title displayed at the bottom of the main screen
    void setLocation(const char* location);
    
    void tickAnimation();
    uint8_t getCurrentScreen() const { return _current_screen; }
};

extern UIManager ui;

#endif // UI_MANAGER_H