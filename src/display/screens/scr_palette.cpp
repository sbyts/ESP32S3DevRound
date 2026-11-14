#include "scr_palette.h"
#include "../display_colors.h"

static void draw_calibration_grid(LGFX_Sprite &spr, const char* title, const DisplayColor* color_array, uint8_t size) {
    spr.clear(TFT_BLACK);
    spr.setFont(&fonts::FreeSansBold12pt7b);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawCenterString(title, 180, 42);
    
    spr.setFont(&fonts::Font0);
    spr.setTextSize(1);

    auto draw_mini_box = [&spr](int32_t x, int32_t y, const DisplayColor &col) {
        spr.fillRect(x, y, 66, 30, col.getRawCode());
        spr.drawRect(x, y, 66, 30, TFT_WHITE);
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.drawCenterString(col.getName().c_str(), x + 33, y + 34);
        spr.setTextColor(spr.color565(130, 130, 130), TFT_BLACK);
        spr.drawCenterString(col.getRgbString().c_str(), x + 33, y + 44);
    };

    int32_t start_x = 56, start_y = 110, spacing_x = 91, spacing_y = 75;
    for (uint8_t i = 0; i < size; i++) {
        int32_t row = i / 3;
        int32_t col = i % 3;
        draw_mini_box(start_x + (col * spacing_x), start_y + (row * spacing_y), color_array[i]);
    }
}

void draw_screen_palette_core(LGFX_Sprite &spr) {
    draw_calibration_grid(spr, "CORE COLORS", palette.core_list, ColorPalette::CORE_SIZE);
}

void draw_screen_palette_design(LGFX_Sprite &spr) {
    draw_calibration_grid(spr, "DESIGN UI COLS", palette.design_list, ColorPalette::DESIGN_SIZE);
}