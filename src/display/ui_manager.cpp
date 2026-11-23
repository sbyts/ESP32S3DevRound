#include "ui_manager.h"
#include "screens/scr_weather.h"
#include "screens/scr_analog_clock.h"
#include "screens/scr_clock.h"
#include "screens/scr_network.h"
#include "screens/scr_palette.h"
#include "screens/scr_lamp.h"
#include "display_colors.h"
#include "../network/lamp_client.h"
#include "../peripherals/sensor_climate.h"

UIManager ui;

uint8_t UIManager::computeTargetBacklight() const {
    if (!climateSensor.isBhReady()) {
        return LCD_BL_MAX;
    }

    float lux = climateSensor.getLux();
    if (lux <= 5.0f) {
        return LCD_BL_MIN;
    } else if (lux <= 50.0f) {
        // Dim room / evening: smooth ramp from LCD_BL_MIN (35) to 110
        float ratio = (lux - 5.0f) / 45.0f;
        return (uint8_t)(LCD_BL_MIN + ratio * (110.0f - LCD_BL_MIN));
    } else if (lux <= 300.0f) {
        // Normal indoor lighting: smooth ramp from 110 to 215
        float ratio = (lux - 50.0f) / 250.0f;
        return (uint8_t)(110.0f + ratio * (215.0f - 110.0f));
    } else if (lux <= 600.0f) {
        // Bright daylight: ramp from 215 to LCD_BL_MAX (255)
        float ratio = (lux - 300.0f) / 300.0f;
        return (uint8_t)(215.0f + ratio * (LCD_BL_MAX - 215.0f));
    }
    return LCD_BL_MAX;
}

void UIManager::updateAutoBrightness(bool instant) {
    if (_is_sleeping) return;

    float target = (float)computeTargetBacklight();
    if (instant) {
        _current_backlight = target;
    } else {
        float diff = target - _current_backlight;
        if (abs(diff) > 0.5f) {
            _current_backlight += diff * 0.06f;
        } else {
            _current_backlight = target;
        }
    }

    uint8_t new_bl = (uint8_t)roundf(_current_backlight);
    if (new_bl != _last_applied_bl) {
        _last_applied_bl = new_bl;
        lcd.setBrightness(new_bl);
    }
}

void UIManager::init() {
    _is_sleeping = false;
    lcd.init();
    lcd.setRotation(0);
    lcd.setBrightness(LCD_BL_MAX);
    _last_applied_bl = LCD_BL_MAX;
    _current_backlight = (float)LCD_BL_MAX;
    lcd.fillScreen(TFT_BLACK);

    palette.init(lcd);

    _spr.setColorDepth(8);
    if (!_spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        Serial.println("ERR: Failed to allocate 8-bit sprite!");
    }

    renderCurrentScreen();
}

void UIManager::renderCurrentScreen() {
    // Must ONLY be called from Core 1 (main loop)
    switch (_current_screen) {
        case 0: {
            float t_in_target, t_out_target, h_in, h_out, w_out;
            char cond_buf[24];
            char loc_buf[32];

            portENTER_CRITICAL(&_mux);
            t_in_target  = _target_indoor_temp;
            t_out_target = _target_outdoor_temp;
            h_in         = _indoor_hum;
            h_out        = _outdoor_hum;
            w_out        = _outdoor_wind;
            strlcpy(cond_buf, _weather_cond, sizeof(cond_buf));
            strlcpy(loc_buf, _location_name, sizeof(loc_buf));
            _outdoor_dirty = false;
            portEXIT_CRITICAL(&_mux);

            draw_screen_weather(_spr, t_in_target, _display_indoor_temp, 
                                t_out_target, _display_outdoor_temp, 
                                h_in, h_out, w_out, 
                                String(cond_buf), String(loc_buf));
            break;
        }
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
    portENTER_CRITICAL(&_mux);
    if (!isnan(temp)) _target_indoor_temp = temp;
    if (!isnan(hum))  _indoor_hum = hum;
    portEXIT_CRITICAL(&_mux);

    if (force_redraw && _current_screen == 0) {
        renderCurrentScreen();
    }
}

void UIManager::setOutdoorData(float temp, float hum, float wind_ms, const char* condition, const char* location) {
    // Thread-safe: safe to call from Core 0 network task.
    // Does NOT trigger SPI flush directly; Core 1 picks up changes via _outdoor_dirty.
    portENTER_CRITICAL(&_mux);
    _target_outdoor_temp = temp;
    _outdoor_hum = hum;
    _outdoor_wind = wind_ms;
    if (condition != nullptr && condition[0] != '\0') {
        strlcpy(_weather_cond, condition, sizeof(_weather_cond));
    }
    if (location != nullptr) {
        strlcpy(_location_name, location, sizeof(_location_name));
    }
    _outdoor_dirty = true;
    portEXIT_CRITICAL(&_mux);
}

void UIManager::setPowerSave(bool sleep) {
    _is_sleeping = sleep;

    if (_is_sleeping) {
        lcd.setBrightness(0);
        _last_applied_bl = 0;
        lcd.fillScreen(TFT_BLACK);
    } else {
        setCpuFrequencyMhz(240);
        lcd.wakeup();
        // Wake directly to the ambient-matched brightness level (no blinding flash at night)
        _last_applied_bl = 0;
        updateAutoBrightness(true);
        renderCurrentScreen();
    }
}

void UIManager::tickAnimation() {
    if (_is_sleeping) return;
    // Limit frame rate to ~60 FPS (every 16 ms)
    if (millis() - _last_anim_time < 16) return;
    _last_anim_time = millis();

    // Smoothly adapt display backlight to ambient room lux
    updateAutoBrightness(false);

    // Screen 0: Smooth interpolation for both Indoor (Outer) and Outdoor (Inner) temperature arcs
    if (_current_screen == 0) {
        portENTER_CRITICAL(&_mux);
        float t_in_target  = _target_indoor_temp;
        float t_out_target = _target_outdoor_temp;
        bool need_redraw   = _outdoor_dirty;
        portEXIT_CRITICAL(&_mux);

        float diff_in = t_in_target - _display_indoor_temp;
        if (abs(diff_in) > 0.05f) {
            _display_indoor_temp += diff_in * 0.12f;
            need_redraw = true;
        } else if (_display_indoor_temp != t_in_target) {
            _display_indoor_temp = t_in_target;
            need_redraw = true;
        }

        float diff_out = t_out_target - _display_outdoor_temp;
        if (abs(diff_out) > 0.05f) {
            _display_outdoor_temp += diff_out * 0.12f;
            need_redraw = true;
        } else if (_display_outdoor_temp != t_out_target) {
            _display_outdoor_temp = t_out_target;
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