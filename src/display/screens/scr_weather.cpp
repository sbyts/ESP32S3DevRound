#include "scr_weather.h"
#include "../display_colors.h"
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

// Helper vector primitives for weather icon builder
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
    // Cut out crescent using black circle offset
    spr.fillCircle(mx + (r / 2), my - (r / 3), r - 1, TFT_BLACK);
    // Tiny twinkling stars
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

// Smart Day/Night & Multi-Condition Icon + Status Label Renderer
static void draw_dynamic_weather_header(LGFX_Sprite &spr, int32_t icon_x, int32_t icon_y, const String &raw_cond)
{
    // Check current hour from internal RTC (Night = 21:00 .. 05:59)
    time_t now;
    time(&now);
    struct tm *t = localtime(&now);
    bool is_night = (t->tm_year > (2020 - 1900)) && (t->tm_hour >= 21 || t->tm_hour < 6);

    String cond = raw_cond;
    cond.toLowerCase();

    String short_label = "CLOUDY";
    uint16_t label_col = palette.yellow.getRawCode();

    // 1. THUNDERSTORM
    if (cond.indexOf("thunder") >= 0 || cond.indexOf("storm") >= 0)
    {
        draw_cloud(spr, icon_x, icon_y - 2, palette.grey.getRawCode());
        // Yellow lightning bolt
        spr.drawLine(icon_x + 1, icon_y + 6, icon_x - 3, icon_y + 13, palette.yellow.getRawCode());
        spr.drawLine(icon_x - 3, icon_y + 13, icon_x + 2, icon_y + 13, palette.yellow.getRawCode());
        spr.drawLine(icon_x + 2, icon_y + 13, icon_x - 2, icon_y + 20, palette.yellow.getRawCode());
        short_label = "STORM";
        label_col = palette.yellow.getRawCode();
    }
    // 2. SNOW / SLEET / ICE
    else if (cond.indexOf("snow") >= 0 || cond.indexOf("sleet") >= 0 || cond.indexOf("blizzard") >= 0 || cond.indexOf("ice") >= 0)
    {
        draw_cloud(spr, icon_x, icon_y - 3, TFT_WHITE);
        // Falling snowflakes
        uint16_t snow_col = palette.cyan.getRawCode();
        spr.fillCircle(icon_x - 6, icon_y + 13, 2, TFT_WHITE);
        spr.fillCircle(icon_x + 1, icon_y + 16, 2, snow_col);
        spr.fillCircle(icon_x + 8, icon_y + 12, 2, TFT_WHITE);
        short_label = "SNOW";
        label_col = palette.cyan.getRawCode();
    }
    // 3. RAIN / DRIZZLE / SHOWER
    else if (cond.indexOf("rain") >= 0 || cond.indexOf("drizzle") >= 0 || cond.indexOf("shower") >= 0)
    {
        draw_cloud(spr, icon_x, icon_y - 3, palette.grey.getRawCode());
        // Diagonal cyan rain drops
        uint16_t rain_col = palette.cyan.getRawCode();
        spr.drawLine(icon_x - 6, icon_y + 10, icon_x - 9, icon_y + 16, rain_col);
        spr.drawLine(icon_x + 1, icon_y + 10, icon_x - 2, icon_y + 16, rain_col);
        spr.drawLine(icon_x + 8, icon_y + 10, icon_x + 5, icon_y + 16, rain_col);
        short_label = "RAIN";
        label_col = palette.cyan.getRawCode();
    }
    // 4. FOG / MIST / HAZE
    else if (cond.indexOf("fog") >= 0 || cond.indexOf("mist") >= 0 || cond.indexOf("haze") >= 0)
    {
        draw_cloud(spr, icon_x, icon_y - 4, palette.grey.getRawCode());
        spr.drawFastHLine(icon_x - 12, icon_y + 10, 24, palette.cyan.getRawCode());
        spr.drawFastHLine(icon_x - 9, icon_y + 14, 18, palette.grey.getRawCode());
        short_label = "FOG";
        label_col = palette.grey.getRawCode();
    }
    // 5. CLEAR / SUNNY (Adapts to Day Sun vs Night Crescent Moon)
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
    // 6. PARTLY CLOUDY (Day Sun+Cloud vs Night Moon+Cloud)
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
    // 7. OVERCAST / CLOUDY (Default)
    else
    {
        draw_cloud(spr, icon_x + 5, icon_y - 4, palette.grey.getRawCode());
        draw_cloud(spr, icon_x - 2, icon_y + 1, TFT_WHITE);
        short_label = "CLOUDY";
        label_col = TFT_WHITE;
    }

    // Draw clean status label next to the icon
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

    // 4. Grid Dividers
    spr.drawFastHLine(100, 198, 160, palette.grey.getRawCode());
    spr.drawFastHLine(100, 232, 160, palette.grey.getRawCode());
    spr.drawFastVLine(185, 238, 46, palette.grey.getRawCode());

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

    // 7. Live Wind Speed Row (in m/s)
    spr.setFont(&fonts::FreeSansBold9pt7b);
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    String wind_label = "WIND: " + String(wind_ms, 1) + " m/s";
    spr.drawCenterString(wind_label.c_str(), 180, 206);

    // 8. Left Bottom Section: Outdoor Temperature from Internet (Integer)
    spr.setTextColor(inner_col, TFT_BLACK);
    String out_t_label = "out: " + String((int)round(target_out_temp)) + "C";
    spr.drawString(out_t_label.c_str(), 100, 252);
    // 9. Right Bottom Section: Stacked Outdoor & Indoor Humidity (Integer)
    spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    String hum_out_label = "out: " + String((int)round(out_hum)) + "%";
    spr.drawString(hum_out_label.c_str(), 195, 240);
    spr.setTextColor(palette.grapefruit.getRawCode(), TFT_BLACK);
    String hum_in_label = "in: " + String((int)round(in_hum)) + "%";
    spr.drawString(hum_in_label.c_str(), 195, 264);
    // 10. Bottom Location Banner
    spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    spr.drawCenterString(location.c_str(), 180, 304);
    spr.setFont(&fonts::Font0);
}