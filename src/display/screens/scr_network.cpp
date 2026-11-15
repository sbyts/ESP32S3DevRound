#include "scr_network.h"
#include "../display_colors.h"
#include "../../network/net_manager.h"
#include "../../system/sys_logger.h"
#include <WiFi.h>

void draw_screen_network(LGFX_Sprite &spr) {
    spr.clear(TFT_BLACK);
    int32_t cx = 180;
    int32_t cy = 180;

    bool connected = (WiFi.status() == WL_CONNECTED);

    // Outer Status Ring
    spr.drawCircle(cx, cy, 172, palette.dark_graphite.getRawCode());
    spr.drawCircle(cx, cy, 171, connected ? palette.cyan.getRawCode() : palette.grapefruit.getRawCode());

    // Header
    spr.setFont(&fonts::FreeSansBold12pt7b);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawCenterString("NET & SYSTEM", cx, 40);

    // Connection Status
    spr.setFont(&fonts::FreeSansBold9pt7b);
    if (connected) {
        spr.setTextColor(palette.lime.getRawCode(), TFT_BLACK);
        spr.drawCenterString("STATUS: ONLINE (STA)", cx, 72);
    } else {
        spr.setTextColor(palette.grapefruit.getRawCode(), TFT_BLACK);
        spr.drawCenterString("STATUS: AP SETUP MODE", cx, 72);
    }

    // Network & Weather Details Block
    spr.setFont(&fonts::Font0);
    spr.setTextSize(1);
    
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawString("NETWORK:", 60, 100);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString(connected ? WiFi.SSID() : "ClimateStation-AP", 125, 100);

    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawString("IP ADDR:", 60, 113);
    spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    spr.drawString(connected ? WiFi.localIP().toString() : WiFi.softAPIP().toString(), 125, 113);

    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawString("WEATHER:", 60, 126);
    String w_stat = net_service.getCity() + " -> " + net_service.getWeatherStatus();
    spr.setTextColor(w_stat.indexOf("OK") >= 0 ? palette.lime.getRawCode() : palette.grapefruit.getRawCode(), TFT_BLACK);
    spr.drawString(w_stat.substring(0, 30), 125, 126);

    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawString("SYSTEM:", 60, 139);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    String sys_str = (connected ? String(WiFi.RSSI()) + " dBm" : "AP") + " | FREE: " + String(ESP.getFreeHeap() / 1024) + " KB";
    spr.drawString(sys_str, 125, 139);

    // Divider before System Logs Area
    spr.drawFastHLine(50, 155, 260, palette.dark_graphite.getRawCode());
    spr.setTextColor(palette.yellow.getRawCode(), TFT_BLACK);
    spr.drawCenterString("LIVE SYSTEM LOGS (LAST 10)", cx, 162);

    // 10-line live console box
    spr.drawRect(46, 175, 268, 124, palette.dark_graphite.getRawCode());
    
    uint8_t total = sys_log.getCount();
    for (uint8_t i = 0; i < total; i++) {
        LogEntry item = sys_log.getEntry(i);
        uint16_t line_col = palette.grey.getRawCode();
        if (item.level == LOG_NET) line_col = palette.cyan.getRawCode();
        else if (item.level == LOG_ERR) line_col = palette.grapefruit.getRawCode();

        spr.setTextColor(line_col, TFT_BLACK);
        // Trim line to 42 chars so it fits cleanly inside the circular display frame
        String trimmed = item.text.substring(0, 42);
        spr.drawString(trimmed, 52, 180 + (i * 11));
    }
}