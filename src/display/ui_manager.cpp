#include "ui_manager.h"
#include "screens/scr_weather.h"
#include "screens/scr_analog_clock.h"
#include "screens/scr_clock.h"
#include "screens/scr_network.h"
#include "screens/scr_palette.h"
#include "screens/scr_lamp.h"
#include "display_colors.h"
#include "../network/lamp_client.h"

UIManager ui;

void UIManager::init() {
    _is_sleeping = false;
    lcd.init();
    lcd.setRotation(0);
    lcd.setBrightness(255);
    lcd.fillScreen(TFT_BLACK);

    palette.init(lcd);

    _spr.setColorDepth(8);
    if (!_spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        Serial.println("ERR: Failed to allocate 8-bit sprite!");
    }

    renderCurrentScreen();
}

void UIManager::renderCurrentScreen() {
    switch (_current_screen) {
        case 0:
            draw_screen_weather(_spr, _target_indoor_temp, _display_indoor_temp, 
                                _target_outdoor_temp, _display_outdoor_temp, 
                                _indoor_hum, _outdoor_hum, _outdoor_wind, 
                                _weather_cond, _location_name);
            break;
        case 1:
            draw_screen_analog_clock(_spr);
            break;
        case 2:
            draw_screen_clock(_spr);
            break;
        case 3:
            draw_screen_network(_spr);
            break;
        case 4:
            draw_screen_palette_core(_spr);
            break;
        case 5:
            draw_screen_palette_design(_spr);
            break;
        case 6:
            draw_screen_lamp(_spr, _display_lamp_bri);
            break;
    }
    _spr.pushSprite(0, 0);
}

void UIManager::nextScreen() {
    _current_screen = (_current_screen + 1) % _total_screens;
    
    if (_current_screen == 0) {
        _display_indoor_temp = -20.0f;
        _display_outdoor_temp = -20.0f;
    } else if (_current_screen == 6) {
        _display_lamp_bri = 0.0f;
    }
    renderCurrentScreen();
}

void UIManager::setIndoorData(float temp, float hum, bool force_redraw) {
    if (!isnan(temp)) _target_indoor_temp = temp;
    if (!isnan(hum))  _indoor_hum = hum;

    if (force_redraw && _current_screen == 0) {
        renderCurrentScreen();
    }
}

void UIManager::setOutdoorData(float temp, float hum, float wind_ms, const char* condition, const char* location) {
    _target_outdoor_temp = temp;
    _outdoor_hum = hum;
    _outdoor_wind = wind_ms;
    if (condition != nullptr && strlen(condition) > 0) {
        _weather_cond = String(condition);
    }
    if (location != nullptr) {
        _location_name = String(location);
    }
    if (_current_screen == 0) {
        renderCurrentScreen();
    }
}

void UIManager::setPowerSave(bool sleep) {
    _is_sleeping = sleep;

    if (_is_sleeping) {
        lcd.setBrightness(0);
        lcd.fillScreen(TFT_BLACK);
    } else {
        setCpuFrequencyMhz(240);
        lcd.wakeup();
        lcd.setBrightness(255);
        renderCurrentScreen();
    }
}

void UIManager::tickAnimation() {
    if (_is_sleeping) return;
    // Limit frame rate to ~60 FPS (every 16 ms)
    if (millis() - _last_anim_time < 16) return;
    _last_anim_time = millis();

    // Screen 0: Smooth interpolation for both Indoor (Outer) and Outdoor (Inner) temperature arcs
    if (_current_screen == 0) {
        bool need_redraw = false;

        float diff_in = _target_indoor_temp - _display_indoor_temp;
        if (abs(diff_in) > 0.05f) {
            _display_indoor_temp += diff_in * 0.12f;
            need_redraw = true;
        } else if (_display_indoor_temp != _target_indoor_temp) {
            _display_indoor_temp = _target_indoor_temp;
            need_redraw = true;
        }

        float diff_out = _target_outdoor_temp - _display_outdoor_temp;
        if (abs(diff_out) > 0.05f) {
            _display_outdoor_temp += diff_out * 0.12f;
            need_redraw = true;
        } else if (_display_outdoor_temp != _target_outdoor_temp) {
            _display_outdoor_temp = _target_outdoor_temp;
            need_redraw = true;
        }

        if (need_redraw) {
            renderCurrentScreen();
        }
    } 
    // Screen 1 (Minimalist Analog Clock) & Screen 2 (Digital Chronograph): continuous 60 FPS sweep
    else if (_current_screen == 1 || _current_screen == 2) {
        renderCurrentScreen();
    }
    // Screen 3 (Network & System Status): periodic refresh once per second
    else if (_current_screen == 3) {
        static unsigned long last_net_draw = 0;
        if (millis() - last_net_draw > 1000) {
            last_net_draw = millis();
            renderCurrentScreen();
        }
    }
    // Screen 6 (Ridy-10-Lite Lamp Link): 60 FPS brightness arc ease + 500 ms countdown refresh
    else if (_current_screen == 6) {
        static unsigned long last_lamp_draw = 0;
        bool need_redraw = false;

        float target_bri = (lampClient.online && lampClient.isOn) ? (float)lampClient.brightness : 0.0f;
        float diff_bri = target_bri - _display_lamp_bri;

        if (abs(diff_bri) > 0.2f) {
            _display_lamp_bri += diff_bri * 0.15f;
            need_redraw = true;
        } else if (_display_lamp_bri != target_bri) {
            _display_lamp_bri = target_bri;
            need_redraw = true;
        }

        if (millis() - last_lamp_draw >= 500) {
            last_lamp_draw = millis();
            need_redraw = true;
        }

        if (need_redraw) {
            renderCurrentScreen();
        }
    }
}