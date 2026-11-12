#ifndef DISPLAY_COLORS_H
#define DISPLAY_COLORS_H

#include <Arduino.h>
#include "display/display_lgfx.h"

// Class representing a single calibrated color with its metadata
class DisplayColor {
private:
    uint16_t _raw_code;    // Hardware-specific 16-bit color code (calibrated BRG)
    String _name;         // String literal name of the color
    String _rgb_string;   // Human-readable string formatted as "R,G,B"

public:
    DisplayColor() : _raw_code(0), _name("UNKNOWN"), _rgb_string("0,0,0") {}

    DisplayColor(uint16_t raw, const char* name_str, const char* rgb_str) {
        _raw_code = raw;
        _name = String(name_str);
        _rgb_string = String(rgb_str);
    }

    uint16_t getRawCode() const { return _raw_code; }
    String getName() const { return _name; }
    String getRgbString() const { return _rgb_string; }
};

// Global palette manager to expose structured colors across all screens
class ColorPalette {
public:
    // Core hardware standard colors
    DisplayColor blue;
    DisplayColor red;
    DisplayColor lime;
    DisplayColor grey;
    DisplayColor forest;

    // Custom curated premium UI colors from the layout image
    DisplayColor orange;
    DisplayColor grapefruit;
    DisplayColor cyan;
    DisplayColor yellow;
    DisplayColor dark_graphite;

    // Partitioned arrays for screen calibration matrices
    static const uint8_t CORE_SIZE = 5;
    static const uint8_t DESIGN_SIZE = 5;
    
    DisplayColor core_list[CORE_SIZE];
    DisplayColor design_list[DESIGN_SIZE];

    // Map and calibrate colors utilizing the active LGFX driver object (BRG correction)
    void init(LGFX_GC9B72 &lcd) {
        // Core standard array
        blue          = DisplayColor(lcd.color565(255, 0, 0),    "BLUE",       "255,0,0");
        red           = DisplayColor(lcd.color565(0, 0, 255),    "RED",        "0,0,255");
        lime          = DisplayColor(lcd.color565(0, 255, 0),    "LIME",       "0,255,0");
        grey          = DisplayColor(lcd.color565(70, 70, 70),   "GREY",       "70,70,70");
        forest        = DisplayColor(lcd.color565(0, 110, 0),    "FOREST",     "0,110,0");

        // UI Premium design array
        orange        = DisplayColor(lcd.color565(0, 150, 255),  "ORANGE",     "0,150,255");
        grapefruit    = DisplayColor(lcd.color565(44, 90, 252),  "GRAPEFRUIT", "252,90,44");
        cyan          = DisplayColor(lcd.color565(255, 190, 0),  "CYAN",       "255,190,0");
        yellow        = DisplayColor(lcd.color565(0, 255, 255),  "YELLOW",     "0,255,255");
        dark_graphite = DisplayColor(lcd.color565(30, 30, 30),   "DK_GREY",    "30,30,30");

        // Pack into structured iterable arrays
        core_list[0] = blue;
        core_list[1] = red;
        core_list[2] = lime;
        core_list[3] = grey;
        core_list[4] = forest;

        design_list[0] = orange;
        design_list[1] = grapefruit;
        design_list[2] = cyan;
        design_list[3] = yellow;
        design_list[4] = dark_graphite;
    }
};

extern ColorPalette palette;

#endif // DISPLAY_COLORS_H