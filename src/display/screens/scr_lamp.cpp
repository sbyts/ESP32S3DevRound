#include "scr_lamp.h"
#include "../display_colors.h"
#include "../../network/lamp_client.h"
#include "../../peripherals/radar_ld2410.h"
#include <math.h>

static uint16_t get_lamp_mode_color(uint8_t mode)
{
    switch (mode)
    {
        case 1: // 3000K Warm -> Signature Grapefruit
            return palette.grapefruit.getRawCode();
        case 2: // 6000K Cool -> Signature Cyan
            return palette.cyan.getRawCode();
        case 3: // 4500K Neutral/Day -> Warm White (BRG calibrated)
        default:
            return lcd.color565(210, 255, 255);
    }
}

static String get_lamp_mode_label(uint8_t mode)
{
    switch (mode)
    {
        case 1: return "3000K WARM";
        case 2: return "6000K COOL";
        case 3: return "4500K DAY";
        default: return "RIDY LAMP";
    }
}

static void draw_lamp_icon(LGFX_Sprite &spr, int32_t ix, int32_t iy, uint16_t col, bool lit)
{
    // Lamp shade dome
    spr.fillCircle(ix, iy, 8, col);
    spr.fillRect(ix - 9, iy + 1, 18, 8, TFT_BLACK);
    spr.fillRoundRect(ix - 9, iy - 1, 18, 4, 2, col);

    // Stem and base
    spr.drawFastVLine(ix, iy + 3, 9, palette.grey.getRawCode());
    spr.drawFastHLine(ix - 5, iy + 12, 11, palette.grey.getRawCode());

    // Light rays when lamp is ON
    if (lit)
    {
        spr.drawLine(ix - 6, iy + 5, ix - 9, iy + 9, col);
        spr.drawLine(ix + 6, iy + 5, ix + 9, iy + 9, col);
        spr.drawLine(ix, iy + 5, ix, iy + 10, col);
    }
}

void draw_screen_lamp(LGFX_Sprite &spr, float disp_bri)
{
    spr.clear(TFT_BLACK);
    int32_t cx = 180;
    int32_t cy = 180;

    uint16_t track_col = palette.dark_graphite.getRawCode();
    uint16_t grey_col  = palette.grey.getRawCode();
    uint16_t mode_col  = get_lamp_mode_color(lampClient.mode);

    // 1. OUTER GAUGE TRACK & ACTIVE ARC: LAMP BRIGHTNESS (0..100%)
    spr.fillArc(cx, cy, 154, 170, 135, 405, track_col);
    spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 162.0f, cy + sin(135 * DEG_TO_RAD) * 162.0f, 8, track_col);
    spr.fillCircle(cx + cos(405 * DEG_TO_RAD) * 162.0f, cy + sin(405 * DEG_TO_RAD) * 162.0f, 8, track_col);

    float bri_clamped = constrain(disp_bri, 0.0f, 100.0f);
    int32_t outer_end_angle = 135 + (int32_t)(270.0f * (bri_clamped / 100.0f));

    if (lampClient.online && bri_clamped > 0.5f && outer_end_angle > 135)
    {
        spr.fillArc(cx, cy, 154, 170, 135, outer_end_angle, mode_col);
        spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 162.0f, cy + sin(135 * DEG_TO_RAD) * 162.0f, 8, mode_col);
        spr.fillCircle(cx + cos(outer_end_angle * DEG_TO_RAD) * 162.0f, cy + sin(outer_end_angle * DEG_TO_RAD) * 162.0f, 8, mode_col);
    }

    // 2. INNER GAUGE TRACK & ACTIVE ARC: RADAR PRESENCE / OFF-DELAY COUNTDOWN
    spr.fillArc(cx, cy, 134, 144, 135, 405, track_col);
    spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 139.0f, cy + sin(135 * DEG_TO_RAD) * 139.0f, 5, track_col);
    spr.fillCircle(cx + cos(405 * DEG_TO_RAD) * 139.0f, cy + sin(405 * DEG_TO_RAD) * 139.0f, 5, track_col);

    float timer_ratio = 0.0f;
    uint16_t timer_col = track_col;

    if (lampClient.online && radar.isEnabled() && lampClient.autoPresence)
    {
        if (lampClient.userPresent)
        {
            timer_ratio = 1.0f;
            timer_col = palette.lime.getRawCode();
        }
        else if (lampClient.isOn && lampClient.offDelaySec > 0 && lampClient.awayElapsedSec < lampClient.offDelaySec)
        {
            timer_ratio = 1.0f - ((float)lampClient.awayElapsedSec / (float)lampClient.offDelaySec);
            timer_col = palette.orange.getRawCode();
        }
    }

    int32_t inner_end_angle = 135 + (int32_t)(270.0f * constrain(timer_ratio, 0.0f, 1.0f));
    if (inner_end_angle > 135)
    {
        spr.fillArc(cx, cy, 134, 144, 135, inner_end_angle, timer_col);
        spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 139.0f, cy + sin(135 * DEG_TO_RAD) * 139.0f, 5, timer_col);
        spr.fillCircle(cx + cos(inner_end_angle * DEG_TO_RAD) * 139.0f, cy + sin(inner_end_angle * DEG_TO_RAD) * 139.0f, 5, timer_col);
    }

    // 3. Speedometer Ticks (matching scr_weather.cpp)
    for (int32_t angle = 135; angle <= 405; angle += 9)
    {
        float rad = angle * DEG_TO_RAD;
        int32_t tick_len = (angle % 45 == 0) ? 8 : 4;
        uint16_t tick_col = (angle % 45 == 0) ? TFT_WHITE : grey_col;
        spr.drawLine(cx + cos(rad) * (134 - tick_len), cy + sin(rad) * (134 - tick_len),
                     cx + cos(rad) * 131, cy + sin(rad) * 131, tick_col);
    }

    // 4. Grid Structural Dividers
    spr.drawFastHLine(100, 198, 160, grey_col);
    spr.drawFastHLine(100, 230, 160, grey_col);
    spr.drawFastHLine(115, 275, 130, grey_col);

    // 5. Header Icon & Lamp Mode Label
    uint16_t icon_col = (lampClient.online && lampClient.isOn) ? mode_col : grey_col;
    draw_lamp_icon(spr, 118, 114, icon_col, lampClient.online && lampClient.isOn);

    spr.setFont(&fonts::FreeSansBold9pt7b);
    spr.setTextSize(1);
    String header_lbl = lampClient.online ? get_lamp_mode_label(lampClient.mode) : "UNREACHABLE";
    uint16_t header_col = lampClient.online ? (lampClient.isOn ? mode_col : grey_col) : palette.red.getRawCode();
    spr.setTextColor(header_col, TFT_BLACK);
    spr.drawString(header_lbl.c_str(), 138, 110);

    // 6. Primary Center Metric: Brightness % / OFF / OFFLINE
    spr.setFont(&fonts::FreeSansBold24pt7b);
    if (!lampClient.online)
    {
        spr.setTextColor(palette.red.getRawCode(), TFT_BLACK);
        spr.drawCenterString("---", cx, 145);
    }
    else if (!lampClient.isOn)
    {
        spr.setTextColor(grey_col, TFT_BLACK);
        spr.drawCenterString("OFF", cx, 145);
    }
    else
    {
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        String bri_str = String((int)round(disp_bri)) + "%";
        spr.drawCenterString(bri_str.c_str(), cx, 145);
    }

    // 7. Middle Row: Radar Link Telemetry
    spr.setFont(&fonts::FreeSansBold9pt7b);
    String radar_row = "RADAR: ";
    if (!radar.isEnabled() || !lampClient.autoPresence)
    {
        radar_row += "MANUAL";
        spr.setTextColor(grey_col, TFT_BLACK);
    }
    else if (lampClient.userPresent)
    {
        radar_row += "DETECTED";
        spr.setTextColor(palette.lime.getRawCode(), TFT_BLACK);
    }
    else
    {
        radar_row += "CLEAR";
        spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    }
    spr.drawCenterString(radar_row.c_str(), cx, 206);

    // 8. Secondary Info Row: Delay Setting & Host
    spr.setTextColor(grey_col, TFT_BLACK);
    String delay_info = "DELAY: " + String(lampClient.offDelaySec) + "s";
    spr.drawCenterString(delay_info.c_str(), cx, 245);

    spr.setFont(&fonts::Font0);
    spr.setTextColor(grey_col, TFT_BLACK);
    spr.drawCenterString("RIDY-10-LITE", cx, 286);

    // 9. Bottom Capsule Pill: Live Presence & Remaining Countdown
    String pill_text = "OFFLINE";
    uint16_t pill_text_col = palette.red.getRawCode();

    if (lampClient.online)
    {
        if (!radar.isEnabled() || !lampClient.autoPresence)
        {
            pill_text = lampClient.isOn ? "LAMP ON" : "STANDBY";
            pill_text_col = lampClient.isOn ? mode_col : grey_col;
        }
        else if (lampClient.userPresent)
        {
            pill_text = "PRESENT";
            pill_text_col = palette.lime.getRawCode();
        }
        else if (lampClient.isOn && lampClient.awayElapsedSec < lampClient.offDelaySec)
        {
            uint32_t rem = lampClient.offDelaySec - lampClient.awayElapsedSec;
            pill_text = "OFF IN " + String(rem) + "s";
            pill_text_col = palette.orange.getRawCode();
        }
        else
        {
            pill_text = "STANDBY";
            pill_text_col = grey_col;
        }
    }

    int32_t pill_w = 136;
    int32_t pill_h = 28;
    int32_t pill_x = cx - (pill_w / 2);
    int32_t pill_y = 315;

    spr.fillRoundRect(pill_x, pill_y, pill_w, pill_h, pill_h / 2, track_col);
    spr.setFont(&fonts::FreeSansBold9pt7b);
    spr.setTextColor(pill_text_col, track_col);
    spr.drawCenterString(pill_text.c_str(), cx, pill_y + 6);
    spr.setFont(&fonts::Font0);
}