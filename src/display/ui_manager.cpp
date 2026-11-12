#include "ui_manager.h"
#include <math.h>

UIManager ui;

void UIManager::init() {
    palette.init(lcd);
    _spr.setColorDepth(16);
    if (_spr.createSprite(lcd.width(), lcd.height()) == nullptr) {
        Serial.println("Error: Failed to allocate hardware canvas frame buffer!");
    }
}

void UIManager::setLocation(const char* location) {
    _location_name = String(location);
    if (_current_screen == 0) {
        draw_screen_premium(_display_temp, _current_hum);
    }
}

void UIManager::nextScreen() {
    _current_screen = (_current_screen + 1) % _total_screens;
    
    // Reset visual gauge to 0 for a smooth startup sweep when returning to the main screen
    if (_current_screen == 0) {
        _display_temp = 0.0f;
    }
    setTargetData(_target_temp, _current_hum, true);
}

void UIManager::setTargetData(float temp, float hum, bool force_redraw) {
    _target_temp = temp;
    _current_hum = hum;

    if (force_redraw) {
        if (_current_screen == 0) {
            draw_screen_premium(_display_temp, _current_hum);
        } else if (_current_screen == 1) {
            draw_screen_palette_core();
        } else {
            draw_screen_palette_design();
        }
    }
}

void UIManager::tickAnimation() {
    if (_current_screen != 0) return;

    // Limit animation frame rate to ~60 FPS (every 16 ms)
    if (millis() - _last_anim_time < 16) return;
    _last_anim_time = millis();

    // Smoothly interpolate visual temperature towards target temperature
    float diff = _target_temp - _display_temp;
    if (abs(diff) > 0.05f) {
        _display_temp += diff * 0.12f;
        draw_screen_premium(_display_temp, _current_hum);
    } else if (_display_temp != _target_temp) {
        _display_temp = _target_temp;
        draw_screen_premium(_display_temp, _current_hum);
    }
}

void UIManager::draw_weather_vector_icon(int32_t x, int32_t y) {
    int32_t sun_x = x + 8;
    int32_t sun_y = y - 6;
    _spr.fillCircle(sun_x, sun_y, 7, palette.yellow.getRawCode());
    
    for (int angle = 0; angle < 360; angle += 45) {
        float rad = angle * DEG_TO_RAD;
        _spr.drawLine(sun_x + cos(rad) * 9,  sun_y + sin(rad) * 9, 
                      sun_x + cos(rad) * 13, sun_y + sin(rad) * 13, palette.yellow.getRawCode());
    }
    _spr.fillCircle(x - 10, y + 4, 7,  TFT_WHITE);
    _spr.fillCircle(x,      y,     10, TFT_WHITE);
    _spr.fillCircle(x + 10, y + 2, 8,  TFT_WHITE);
    _spr.fillRect(x - 10, y + 4, 20, 7, TFT_WHITE);
}

void UIManager::draw_screen_premium(float temp, float hum) {
    _spr.clear(TFT_BLACK);
    int32_t cx = 180;
    int32_t cy = 180;

    // 1. Outer Grapefruit Gauge Ring with rounded caps
    _spr.fillArc(cx, cy, 154, 170, 135, 405, palette.dark_graphite.getRawCode());
    _spr.fillArc(cx, cy, 154, 170, 135, 235, palette.grapefruit.getRawCode());
    _spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 162.0f, cy + sin(135 * DEG_TO_RAD) * 162.0f, 8, palette.grapefruit.getRawCode());
    _spr.fillCircle(cx + cos(235 * DEG_TO_RAD) * 162.0f, cy + sin(235 * DEG_TO_RAD) * 162.0f, 8, palette.grapefruit.getRawCode());

    // 2. Inner Active Cyan Gauge Ring with smooth animated angle
    _spr.fillArc(cx, cy, 134, 144, 135, 405, palette.dark_graphite.getRawCode());
    float current_temp = constrain(temp, 0.0f, 50.0f);
    int32_t end_angle = 135 + (int32_t)(270.0f * (current_temp / 50.0f));
    
    if (end_angle > 135) {
        _spr.fillArc(cx, cy, 134, 144, 135, end_angle, palette.cyan.getRawCode());
        _spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 139.0f, cy + sin(135 * DEG_TO_RAD) * 139.0f, 5, palette.cyan.getRawCode());
        _spr.fillCircle(cx + cos(end_angle * DEG_TO_RAD) * 139.0f, cy + sin(end_angle * DEG_TO_RAD) * 139.0f, 5, palette.cyan.getRawCode());
    }

    // 3. Speedometer-style Ticks
    for (int32_t angle = 135; angle <= 405; angle += 9) {
        float rad = angle * DEG_TO_RAD;
        int32_t tick_len = (angle % 45 == 0) ? 8 : 4;
        uint16_t tick_col = (angle % 45 == 0) ? TFT_WHITE : palette.grey.getRawCode();
        _spr.drawLine(cx + cos(rad) * (134 - tick_len), cy + sin(rad) * (134 - tick_len), 
                      cx + cos(rad) * 131,              cy + sin(rad) * 131, tick_col);
    }

    // 4. Grid Structural Dividers (Vertical divider line included)
    _spr.drawFastHLine(100, 198, 160, palette.grey.getRawCode());
    _spr.drawFastHLine(100, 232, 160, palette.grey.getRawCode());
    _spr.drawFastVLine(185, 238, 46,  palette.grey.getRawCode());

    // 5. Anti-Aliased Smooth Fonts Rendering
    draw_weather_vector_icon(132, 118);
    
    _spr.setFont(&fonts::FreeSansBold9pt7b);
    _spr.setTextSize(1);
    _spr.setTextColor(palette.yellow.getRawCode(), TFT_BLACK);
    _spr.drawString("CLOUDY", 170, 110);

    // Dynamic spacing calculation for Temperature + Degree Symbol + "C"
    _spr.setFont(&fonts::FreeSansBold24pt7b);
    _spr.setTextColor(TFT_WHITE, TFT_BLACK);
    
    char num_buf[16];
    snprintf(num_buf, sizeof(num_buf), "%.0f", _target_temp);
    
    int32_t num_w = _spr.textWidth(num_buf);
    int32_t c_w   = _spr.textWidth("C");
    int32_t deg_space = 14; 
    int32_t total_w = num_w + deg_space + c_w;
    int32_t start_x = cx - (total_w / 2);

    _spr.drawString(num_buf, start_x, 145);
    
    int32_t deg_x = start_x + num_w + 6;
    _spr.drawCircle(deg_x, 152, 4, TFT_WHITE);
    _spr.drawCircle(deg_x, 152, 3, TFT_WHITE);

    _spr.drawString("C", start_x + num_w + deg_space, 145);

    // Secondary telemetry rows
    _spr.setFont(&fonts::FreeSansBold9pt7b);
    _spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    _spr.drawCenterString("WIND: 14.5 m/s", 180, 206);

    // Right Column Stacked Humidity Layout (Aligned to X = 195 right after vertical line)
    _spr.setTextColor(palette.grapefruit.getRawCode(), TFT_BLACK);
    char hum_out_buf[16];
    snprintf(hum_out_buf, sizeof(hum_out_buf), "out: %.0f%%", (hum > 0) ? hum : 78.0f);
    _spr.drawString(hum_out_buf, 195, 240);

    _spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    char hum_in_buf[16];
    snprintf(hum_in_buf, sizeof(hum_in_buf), "in: %.0f%%", hum);
    _spr.drawString(hum_in_buf, 195, 264);

    // Bottom Location Banner
    _spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    _spr.drawCenterString(_location_name.c_str(), 180, 304);

    _spr.setFont(&fonts::Font0);
    _spr.pushSprite(0, 0);
}

void UIManager::draw_calibration_grid(const char* title, const DisplayColor* color_array, uint8_t size) {
    _spr.clear(TFT_BLACK);
    _spr.setFont(&fonts::FreeSansBold12pt7b);
    _spr.setTextColor(TFT_WHITE, TFT_BLACK);
    _spr.drawCenterString(title, 180, 42);
    
    _spr.setFont(&fonts::Font0);
    _spr.setTextSize(1);

    auto draw_mini_box = [this](int32_t x, int32_t y, const DisplayColor &col) {
        _spr.fillRect(x, y, 66, 30, col.getRawCode());
        _spr.drawRect(x, y, 66, 30, TFT_WHITE);
        _spr.setTextColor(TFT_WHITE, TFT_BLACK);
        _spr.drawCenterString(col.getName().c_str(), x + 33, y + 34);
        _spr.setTextColor(_spr.color565(130, 130, 130), TFT_BLACK);
        _spr.drawCenterString(col.getRgbString().c_str(), x + 33, y + 44);
    };

    int32_t start_x = 56, start_y = 110, spacing_x = 91, spacing_y = 75;
    for (uint8_t i = 0; i < size; i++) {
        int32_t row = i / 3;
        int32_t col = i % 3;
        draw_mini_box(start_x + (col * spacing_x), start_y + (row * spacing_y), color_array[i]);
    }
    _spr.pushSprite(0, 0);
}

void UIManager::draw_screen_palette_core() {
    draw_calibration_grid("CORE COLORS", palette.core_list, ColorPalette::CORE_SIZE);
}

void UIManager::draw_screen_palette_design() {
    draw_calibration_grid("DESIGN UI COLS", palette.design_list, ColorPalette::DESIGN_SIZE);
}