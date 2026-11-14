#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include "display/display_lgfx.h"
#include "display/display_colors.h"

class UIManager {
private:
    LGFX_Sprite _spr;
    uint8_t _current_screen;
    const uint8_t _total_screens;

    float _target_indoor_temp;
    float _display_indoor_temp;
    float _indoor_hum;

    float _target_outdoor_temp;
    float _display_outdoor_temp;
    float _outdoor_hum;
    float _outdoor_wind;

    String _weather_cond;
    String _location_name;
    unsigned long _last_anim_time;

    void renderCurrentScreen();

public:
    UIManager() : _spr(&lcd), _current_screen(0), _total_screens(6), 
                  _target_indoor_temp(23.0f), _display_indoor_temp(-20.0f), _indoor_hum(45.0f),
                  _target_outdoor_temp(18.0f), _display_outdoor_temp(-20.0f), _outdoor_hum(78.0f),
                  _outdoor_wind(0.0f), _weather_cond("CLOUDY"), _location_name("KYIV"), _last_anim_time(0) {}

    void init();
    void nextScreen();
    
    void setIndoorData(float temp, float hum, bool force_redraw = false);
    void setOutdoorData(float temp, float hum, float wind_ms, const char* condition = nullptr, const char* location = nullptr);
    
    void tickAnimation();
    uint8_t getCurrentScreen() const { return _current_screen; }
};

extern UIManager ui;

#endif // UI_MANAGER_H