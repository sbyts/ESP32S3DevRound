#include "scr_clock.h"
#include "../display_colors.h"
#include <sys/time.h>
#include <time.h>
#include <math.h>

void draw_screen_clock(LGFX_Sprite &spr) {
    spr.clear(TFT_BLACK);
    int32_t cx = 180;
    int32_t cy = 180;

    // 1. Get unified high-precision time (seconds + microseconds from a single RTC source)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *timeinfo = localtime(&tv.tv_sec);
    
    bool has_ntp = (timeinfo->tm_year > (2020 - 1900));

    // Calculate uptime stopwatch independently using millis()
    unsigned long total_ms = millis();
    uint32_t uptime_sec = total_ms / 1000;
    uint8_t sw_sec = uptime_sec % 60;
    uint8_t sw_min = (uptime_sec / 60) % 60;
    uint8_t sw_tenths = (total_ms % 1000) / 100;

    uint8_t disp_hour = has_ntp ? timeinfo->tm_hour : ((14 + (uptime_sec / 3600)) % 24);
    uint8_t disp_min  = has_ntp ? timeinfo->tm_min  : sw_min;

    // Exact continuous seconds (0.000000 to 59.999999) without phase mismatch!
    float exact_sec = timeinfo->tm_sec + (tv.tv_usec / 1000000.0f);

    // 2. Outer Full-Circle Seconds Progress Ring (silky-smooth 60 FPS sweep)
    spr.fillArc(cx, cy, 162, 172, 0, 360, palette.dark_graphite.getRawCode());
    float exact_angle = 360.0f * (exact_sec / 60.0f);
    
    if (exact_angle > 0.5f) {
        spr.fillArc(cx, cy, 162, 172, 270, 270 + (int32_t)exact_angle, palette.cyan.getRawCode());
    }
    
    // Use exact floating-point angle for the white dot so it glides sub-pixel smoothly
    float lead_rad = (270.0f + exact_angle) * DEG_TO_RAD;
    spr.fillCircle(cx + cos(lead_rad) * 167.0f, cy + sin(lead_rad) * 167.0f, 6, TFT_WHITE);

    // 3. Dial Hour Markers (12 ticks)
    for (int i = 0; i < 12; i++) {
        float rad = (i * 30) * DEG_TO_RAD;
        int32_t len = (i % 3 == 0) ? 12 : 6;
        uint16_t col = (i % 3 == 0) ? palette.grapefruit.getRawCode() : palette.grey.getRawCode();
        spr.drawLine(cx + cos(rad) * (152 - len), cy + sin(rad) * (152 - len),
                     cx + cos(rad) * 152,         cy + sin(rad) * 152, col);
    }

    // 4. Top Header Badge
    spr.setFont(&fonts::FreeSansBold9pt7b);
    spr.setTextColor(has_ntp ? palette.cyan.getRawCode() : palette.grey.getRawCode(), TFT_BLACK);
    spr.drawCenterString(has_ntp ? "NTP SYNCED" : "LOCAL CLOCK", cx, 85);

    // 5. Center Digital Time (HH:MM) using safe String formatting
    spr.setFont(&fonts::FreeSansBold24pt7b);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    String time_str = (disp_hour < 10 ? "0" : "") + String(disp_hour) + ":" + 
                      (disp_min < 10 ? "0" : "") + String(disp_min);
    spr.drawCenterString(time_str.c_str(), cx, 130);

    spr.drawFastHLine(100, 195, 160, palette.dark_graphite.getRawCode());

    // 6. Live Uptime Stopwatch Counter (MM:SS.d)
    spr.setFont(&fonts::FreeSansBold12pt7b);
    spr.setTextColor(palette.grapefruit.getRawCode(), TFT_BLACK);
    String sw_str = (sw_min < 10 ? "0" : "") + String(sw_min) + ":" + 
                    (sw_sec < 10 ? "0" : "") + String(sw_sec) + "." + String(sw_tenths);
    spr.drawCenterString(sw_str.c_str(), cx, 212);

    // 7. Bottom Real Date String
    spr.setFont(&fonts::FreeSansBold9pt7b);
    spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    
    if (has_ntp) {
        const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
        const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        String date_str = String(days[timeinfo->tm_wday]) + " " + 
                          String(timeinfo->tm_mday) + " " + 
                          String(months[timeinfo->tm_mon]) + " " + 
                          String(timeinfo->tm_year + 1900);
        spr.drawCenterString(date_str.c_str(), cx, 275);
    } else {
        spr.drawCenterString("WAITING FOR WI-FI", cx, 275);
    }

    spr.setFont(&fonts::Font0);
}