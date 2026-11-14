#include "scr_analog_clock.h"
#include "../display_colors.h"
#include <sys/time.h>
#include <time.h>
#include <math.h>

// Draws a sleek pill-shaped (capsule) line with rounded ends between two points
static void draw_rounded_capsule(LGFX_Sprite &spr, float x1, float y1, float x2, float y2, 
                                 float thickness, uint16_t color) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrt(dx * dx + dy * dy);
    if (len < 0.1f) return;

    float radius = thickness / 2.0f;
    float nx = -dy / len * radius;
    float ny =  dx / len * radius;

    // Rectangular body
    spr.fillTriangle(round(x1 + nx), round(y1 + ny), round(x2 + nx), round(y2 + ny), round(x2 - nx), round(y2 - ny), color);
    spr.fillTriangle(round(x1 + nx), round(y1 + ny), round(x2 - nx), round(y2 - ny), round(x1 - nx), round(y1 - ny), color);

    // Smooth rounded end caps
    spr.fillCircle(round(x1), round(y1), round(radius), color);
    spr.fillCircle(round(x2), round(y2), round(radius), color);
}

void draw_screen_analog_clock(LGFX_Sprite &spr) {
    spr.clear(TFT_BLACK);
    int32_t cx = 180;
    int32_t cy = 180;

    // Fetch current time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *timeinfo = localtime(&tv.tv_sec);
    bool has_ntp = (timeinfo->tm_year > (2020 - 1900));

    unsigned long uptime_sec = millis() / 1000;
    uint8_t hour = has_ntp ? (timeinfo->tm_hour % 12) : ((10 + (uptime_sec / 3600)) % 12);
    uint8_t min  = has_ntp ? timeinfo->tm_min : ((10 + (uptime_sec / 60)) % 60);
    uint8_t sec  = has_ntp ? timeinfo->tm_sec : (uptime_sec % 60);

    // 1. Minimalist 12 Hour Index Markers (Cardinal ticks in brighter dark_graphite, minor ticks in grey)
    for (int i = 0; i < 12; i++) {
        float rad = ((i * 30.0f) - 90.0f) * DEG_TO_RAD;
        bool is_cardinal = (i % 3 == 0); // 12, 3, 6, 9 o'clock
        
        float outer_r = 164.0f;
        float inner_r = is_cardinal ? 142.0f : 152.0f;
        float thick   = is_cardinal ? 6.0f : 3.0f;
        uint16_t col  = is_cardinal ? palette.dark_graphite.getRawCode() : palette.grey.getRawCode();

        float x1 = cx + cos(rad) * inner_r;
        float y1 = cy + sin(rad) * inner_r;
        float x2 = cx + cos(rad) * outer_r;
        float y2 = cy + sin(rad) * outer_r;

        draw_rounded_capsule(spr, x1, y1, x2, y2, thick, col);
    }

    // 2. Smooth angles for Hour and Minute hands
    float min_angle  = (min + (sec / 60.0f)) * 6.0f;
    float hour_angle = (hour + (min / 60.0f)) * 30.0f;

    float hour_rad = (hour_angle - 90.0f) * DEG_TO_RAD;
    float min_rad  = (min_angle - 90.0f) * DEG_TO_RAD;

    // 3. Hour Hand: Thick Cyan rounded capsule with subtle counterweight
    float hx1 = cx - cos(hour_rad) * 18.0f;
    float hy1 = cy - sin(hour_rad) * 18.0f;
    float hx2 = cx + cos(hour_rad) * 86.0f;
    float hy2 = cy + sin(hour_rad) * 86.0f;
    draw_rounded_capsule(spr, hx1, hy1, hx2, hy2, 12.0f, palette.cyan.getRawCode());

    // 4. Minute Hand: Sleek Grapefruit rounded capsule layered cleanly on top
    float mx1 = cx - cos(min_rad) * 22.0f;
    float my1 = cy - sin(min_rad) * 22.0f;
    float mx2 = cx + cos(min_rad) * 128.0f;
    float my2 = cy + sin(min_rad) * 128.0f;
    draw_rounded_capsule(spr, mx1, my1, mx2, my2, 9.0f, palette.grapefruit.getRawCode());
}