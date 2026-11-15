#include "scr_weather.h"
#include "../display_colors.h"
#include <sys/time.h>
#include <time.h>
#include <math.h>

static uint16_t get_temp_color(float temp, bool is_indoor)
{
    if (temp < 0.0f)
    {
        return palette.blue.getRawCode();
    }
    else if (temp <= 15.0f)
    {
        return palette.cyan.getRawCode();
    }
    else if (temp <= 26.0f)
    {
        return is_indoor ? palette.grapefruit.getRawCode() : palette.lime.getRawCode();
    }
    else
    {
        return palette.red.getRawCode();
    }
}

static void draw_sun(LGFX_Sprite &spr, int32_t sx, int32_t sy, int32_t r)
{
    spr.fillCircle(sx, sy, r, palette.yellow.getRawCode());
    for (int angle = 0; angle < 360; angle += 45)
    {
        float rad = angle * DEG_TO_RAD;
        spr.drawLine(sx + cos(rad) * (r + 2), sy + sin(rad) * (r + 2),
                     sx + cos(rad) * (r + 6), sy + sin(rad) * (r + 6), palette.yellow.getRawCode());
    }
}

static void draw_moon(LGFX_Sprite &spr, int32_t mx, int32_t my, int32_t r)
{
    spr.fillCircle(mx, my, r, palette.yellow.getRawCode());
    spr.fillCircle(mx + (r / 2), my - (r / 3), r - 1, TFT_BLACK);
    spr.drawPixel(mx - r - 3, my - 4, TFT_WHITE);
    spr.drawPixel(mx + r + 2, my + 5, palette.cyan.getRawCode());
}

static void draw_cloud(LGFX_Sprite &spr, int32_t x, int32_t y, uint16_t color)
{
    spr.fillCircle(x - 10, y + 4, 7, color);
    spr.fillCircle(x, y, 10, color);
    spr.fillCircle(x + 10, y + 2, 8, color);
    spr.fillRect(x - 10, y + 4, 20, 7, color);
}

static void draw_dynamic_weather_header(LGFX_Sprite &spr, int32_t icon_x, int32_t icon_y, const String &raw_cond)
{
    time_t now;
    time(&now);
    struct tm *t = localtime(&now);
    bool is_night = (t->tm_year > (2020 - 1900)) && (t->tm_hour >= 21 || t->tm_hour < 6);

    String cond = raw_cond;
    cond.toLowerCase();

    String short_label = "CLOUDY";
    uint16_t label_col = palette.yellow.getRawCode();

    if (cond.indexOf("thunder") >= 0 || cond.indexOf("storm") >= 0)
    {
        draw_cloud(spr, icon_x, icon_y - 2, palette.grey.getRawCode());
        spr.drawLine(icon_x + 1, icon_y + 6, icon_x - 3, icon_y + 13, palette.yellow.getRawCode());
        spr.drawLine(icon_x - 3, icon_y + 13, icon_x + 2, icon_y + 13, palette.yellow.getRawCode());
        spr.drawLine(icon_x + 2, icon_y + 13, icon_x - 2, icon_y + 20, palette.yellow.getRawCode());
        short_label = "STORM";
        label_col = palette.yellow.getRawCode();
    }
    else if (cond.indexOf("snow") >= 0 || cond.indexOf("sleet") >= 0 || cond.indexOf("blizzard") >= 0 || cond.indexOf("ice") >= 0)
    {
        draw_cloud(spr, icon_x, icon_y - 3, TFT_WHITE);
        uint16_t snow_col = palette.cyan.getRawCode();
        spr.fillCircle(icon_x - 6, icon_y + 13, 2, TFT_WHITE);
        spr.fillCircle(icon_x + 1, icon_y + 16, 2, snow_col);
        spr.fillCircle(icon_x + 8, icon_y + 12, 2, TFT_WHITE);
        short_label = "SNOW";
        label_col = palette.cyan.getRawCode();
    }
    else if (cond.indexOf("rain") >= 0 || cond.indexOf("drizzle") >= 0 || cond.indexOf("shower") >= 0)
    {
        draw_cloud(spr, icon_x, icon_y - 3, palette.grey.getRawCode());
        uint16_t rain_col = palette.cyan.getRawCode();
        spr.drawLine(icon_x - 6, icon_y + 10, icon_x - 9, icon_y + 16, rain_col);
        spr.drawLine(icon_x + 1, icon_y + 10, icon_x - 2, icon_y + 16, rain_col);
        spr.drawLine(icon_x + 8, icon_y + 10, icon_x + 5, icon_y + 16, rain_col);
        short_label = "RAIN";
        label_col = palette.cyan.getRawCode();
    }
    else if (cond.indexOf("fog") >= 0 || cond.indexOf("mist") >= 0 || cond.indexOf("haze") >= 0)
    {
        draw_cloud(spr, icon_x, icon_y - 4, palette.grey.getRawCode());
        spr.drawFastHLine(icon_x - 12, icon_y + 10, 24, palette.cyan.getRawCode());
        spr.drawFastHLine(icon_x - 9, icon_y + 14, 18, palette.grey.getRawCode());
        short_label = "FOG";
        label_col = palette.grey.getRawCode();
    }
    else if (cond.indexOf("clear") >= 0 || cond.indexOf("sunny") >= 0)
    {
        if (is_night)
        {
            draw_moon(spr, icon_x, icon_y + 1, 9);
            short_label = "CLEAR";
            label_col = palette.cyan.getRawCode();
        }
        else
        {
            draw_sun(spr, icon_x, icon_y + 1, 8);
            short_label = "SUNNY";
            label_col = palette.yellow.getRawCode();
        }
    }
    else if (cond.indexOf("partly") >= 0)
    {
        if (is_night)
        {
            draw_moon(spr, icon_x + 8, icon_y - 6, 7);
            short_label = "NIGHT";
            label_col = palette.cyan.getRawCode();
        }
        else
        {
            draw_sun(spr, icon_x + 8, icon_y - 6, 7);
            short_label = "PARTLY";
            label_col = palette.yellow.getRawCode();
        }
        draw_cloud(spr, icon_x, icon_y, TFT_WHITE);
    }
    else
    {
        draw_cloud(spr, icon_x + 5, icon_y - 4, palette.grey.getRawCode());
        draw_cloud(spr, icon_x - 2, icon_y + 1, TFT_WHITE);
        short_label = "CLOUDY";
        label_col = TFT_WHITE;
    }

    spr.setFont(&fonts::FreeSansBold9pt7b);
    spr.setTextSize(1);
    spr.setTextColor(label_col, TFT_BLACK);
    spr.drawString(short_label.c_str(), 168, 110);
}

void draw_screen_weather(LGFX_Sprite &spr, float target_in_temp, float disp_in_temp,
                         float target_out_temp, float disp_out_temp,
                         float in_hum, float out_hum, float wind_ms,
                         const String &condition, const String &location)
{
    spr.clear(TFT_BLACK);
    int32_t cx = 180;
    int32_t cy = 180;

    uint16_t track_col = palette.dark_graphite.getRawCode();

    // 1. OUTER GAUGE TRACK & ACTIVE ARC: INDOOR TEMPERATURE (-20C to +40C)
    spr.fillArc(cx, cy, 154, 170, 135, 405, track_col);
    spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 162.0f, cy + sin(135 * DEG_TO_RAD) * 162.0f, 8, track_col);
    spr.fillCircle(cx + cos(405 * DEG_TO_RAD) * 162.0f, cy + sin(405 * DEG_TO_RAD) * 162.0f, 8, track_col);

    float in_t_mapped = constrain(disp_in_temp + 20.0f, 0.0f, 60.0f);
    int32_t outer_end_angle = 135 + (int32_t)(270.0f * (in_t_mapped / 60.0f));
    uint16_t outer_col = get_temp_color(target_in_temp, true);

    if (outer_end_angle > 135)
    {
        spr.fillArc(cx, cy, 154, 170, 135, outer_end_angle, outer_col);
        spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 162.0f, cy + sin(135 * DEG_TO_RAD) * 162.0f, 8, outer_col);
        spr.fillCircle(cx + cos(outer_end_angle * DEG_TO_RAD) * 162.0f, cy + sin(outer_end_angle * DEG_TO_RAD) * 162.0f, 8, outer_col);
    }

    // 2. INNER GAUGE TRACK & ACTIVE ARC: OUTDOOR TEMPERATURE (-20C to +40C)
    spr.fillArc(cx, cy, 134, 144, 135, 405, track_col);
    spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 139.0f, cy + sin(135 * DEG_TO_RAD) * 139.0f, 5, track_col);
    spr.fillCircle(cx + cos(405 * DEG_TO_RAD) * 139.0f, cy + sin(405 * DEG_TO_RAD) * 139.0f, 5, track_col);

    float out_t_mapped = constrain(disp_out_temp + 20.0f, 0.0f, 60.0f);
    int32_t inner_end_angle = 135 + (int32_t)(270.0f * (out_t_mapped / 60.0f));
    uint16_t inner_col = get_temp_color(target_out_temp, false);

    if (inner_end_angle > 135)
    {
        spr.fillArc(cx, cy, 134, 144, 135, inner_end_angle, inner_col);
        spr.fillCircle(cx + cos(135 * DEG_TO_RAD) * 139.0f, cy + sin(135 * DEG_TO_RAD) * 139.0f, 5, inner_col);
        spr.fillCircle(cx + cos(inner_end_angle * DEG_TO_RAD) * 139.0f, cy + sin(inner_end_angle * DEG_TO_RAD) * 139.0f, 5, inner_col);
    }

    // 3. Speedometer Ticks
    for (int32_t angle = 135; angle <= 405; angle += 9)
    {
        float rad = angle * DEG_TO_RAD;
        int32_t tick_len = (angle % 45 == 0) ? 8 : 4;
        uint16_t tick_col = (angle == 225) ? palette.cyan.getRawCode() : ((angle % 45 == 0) ? TFT_WHITE : palette.grey.getRawCode());
        spr.drawLine(cx + cos(rad) * (134 - tick_len), cy + sin(rad) * (134 - tick_len),
                     cx + cos(rad) * 131, cy + sin(rad) * 131, tick_col);
    }

    // 4. Grid Structural Dividers (Adjusted bottom spacing to give the clock pill breathing room)
    spr.drawFastHLine(100, 198, 160, palette.grey.getRawCode()); // Top Wind line
    spr.drawFastHLine(100, 230, 160, palette.grey.getRawCode()); // Bottom Wind line
    spr.drawFastVLine(185, 234, 38, palette.grey.getRawCode());  // Center split between Temp & Humidity
    spr.drawFastHLine(115, 275, 130, palette.grey.getRawCode()); // Top Location line
    spr.drawFastHLine(130, 303, 100, palette.grey.getRawCode()); // Bottom Location line

    // 5. Dynamic Day/Night Weather Icon & Status Label
    draw_dynamic_weather_header(spr, 132, 118, condition);

    // 6. Primary Center Metric: INDOOR SENSOR TEMPERATURE (Integer)
    spr.setFont(&fonts::FreeSansBold24pt7b);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);

    String num_str = String((int)round(target_in_temp));
    int32_t num_w = spr.textWidth(num_str.c_str());
    int32_t c_w = spr.textWidth("C");
    int32_t deg_space = 14;
    int32_t total_w = num_w + deg_space + c_w;
    int32_t start_x = cx - (total_w / 2);

    spr.drawString(num_str.c_str(), start_x, 145);
    int32_t deg_x = start_x + num_w + 6;
    spr.drawCircle(deg_x, 152, 4, TFT_WHITE);
    spr.drawCircle(deg_x, 152, 3, TFT_WHITE);
    spr.drawString("C", start_x + num_w + deg_space, 145);

    // 7. Live Wind Speed Row
    spr.setFont(&fonts::FreeSansBold9pt7b);
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    String wind_label = "WIND: " + String(wind_ms, 1) + " m/s";
    spr.drawCenterString(wind_label.c_str(), cx, 206);

    // 8. Left Bottom Section: Pure Outdoor Temperature centered in left cell (X = 138)
    int out_int = (int)round(target_out_temp);
    String signed_temp = (out_int > 0 ? "+" : "") + String(out_int);

    spr.setFont(&fonts::FreeSansBold12pt7b);
    int32_t val_w = spr.textWidth(signed_temp.c_str());
    spr.setFont(&fonts::FreeSansBold9pt7b);
    int32_t unit_w = 10 + spr.textWidth("C");
    int32_t cur_x = 138 - ((val_w + unit_w) / 2);
    int32_t base_y = 246;
    uint16_t cyan_col = palette.cyan.getRawCode();
    spr.setFont(&fonts::FreeSansBold12pt7b);
    spr.setTextColor(cyan_col, TFT_BLACK);
    spr.drawString(signed_temp.c_str(), cur_x, base_y - 2);
    cur_x += val_w;
    spr.setFont(&fonts::FreeSansBold9pt7b);
    spr.drawCircle(cur_x + 3, base_y + 3, 2, cyan_col);
    spr.setTextColor(cyan_col, TFT_BLACK);
    spr.drawString("C", cur_x + 7, base_y + 2);
    // 9. Right Bottom Section: Stacked Outdoor & Indoor Humidity (X = 196, Y = 236 & 256)
    spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    String hum_out_label = "out: " + String((int)round(out_hum)) + "%";
    spr.drawString(hum_out_label.c_str(), 196, 236);
    spr.setTextColor(palette.grapefruit.getRawCode(), TFT_BLACK);
    String hum_in_label = "in: " + String((int)round(in_hum)) + "%";
    spr.drawString(hum_in_label.c_str(), 196, 256);
    // 10. Location Banner (Grey color, centered in 275..303 row -> Y = 281)
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawCenterString(location.c_str(), cx, 281);
    // 11. Digital Clock inside a Dark Graphite Rounded Pill Badge (matching arc end-caps)
    time_t now;
    time(&now);
    struct tm *t = localtime(&now);
    bool has_ntp = (t->tm_year > (2020 - 1900));
    uint8_t h = has_ntp ? t->tm_hour : ((14 + (millis() / 3600000)) % 24);
    uint8_t m = has_ntp ? t->tm_min : ((millis() / 60000) % 60);
    String clock_str = (h < 10 ? "0" : "") + String(h) + ":" + (m < 10 ? "0" : "") + String(m);
    int32_t pill_w = 94;
    int32_t pill_h = 28;
    int32_t pill_x = cx - (pill_w / 2);
    int32_t pill_y = 315;
    uint16_t pill_bg = palette.dark_graphite.getRawCode();
    // Rounded capsule background (radius = pill_h / 2 = 14)
    spr.fillRoundRect(pill_x, pill_y, pill_w, pill_h, pill_h / 2, pill_bg);
    // Crisp grey time digits centered vertically inside the graphite pill
    spr.setFont(&fonts::FreeSansBold12pt7b);
    spr.setTextColor(palette.grey.getRawCode(), pill_bg);
    spr.drawCenterString(clock_str.c_str(), cx, pill_y + 5);
    spr.setFont(&fonts::Font0);
}