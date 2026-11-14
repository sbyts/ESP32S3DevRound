#ifndef SCR_WEATHER_H
#define SCR_WEATHER_H

#include <Arduino.h>
#include "../display_lgfx.h"

void draw_screen_weather(LGFX_Sprite &spr, float target_in_temp, float disp_in_temp, 
                         float target_out_temp, float disp_out_temp, 
                         float in_hum, float out_hum, float wind_ms, 
                         const String &condition, const String &location);

#endif // SCR_WEATHER_H