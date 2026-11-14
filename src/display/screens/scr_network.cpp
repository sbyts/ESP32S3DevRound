#include "scr_network.h"
#include "../display_colors.h"
#include "../../network/net_manager.h"
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
    spr.drawCenterString("NET & SYSTEM", cx, 42);

    // Connection Status
    spr.setFont(&fonts::FreeSansBold9pt7b);
    if (connected) {
        spr.setTextColor(palette.lime.getRawCode(), TFT_BLACK);
        spr.drawCenterString("STATUS: ONLINE (STA)", cx, 78);
    } else {
        spr.setTextColor(palette.grapefruit.getRawCode(), TFT_BLACK);
        spr.drawCenterString("STATUS: AP SETUP MODE", cx, 78);
    }

    // Network & Weather Details Block
    spr.setFont(&fonts::Font0);
    spr.setTextSize(1);
    
    // Row 1: Network
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawString("NETWORK:", 60, 108);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString(connected ? WiFi.SSID() : "ClimateStation-AP", 125, 108);

    // Row 2: IP Address
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawString("IP ADDR:", 60, 122);
    spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
    spr.drawString(connected ? WiFi.localIP().toString() : WiFi.softAPIP().toString(), 125, 122);

    // Row 3: City & Weather API Status
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawString("WEATHER:", 60, 136);
    String w_stat = net_service.getCity() + " -> " + net_service.getWeatherStatus();
    spr.setTextColor(w_stat.indexOf("OK") >= 0 ? palette.lime.getRawCode() : palette.grapefruit.getRawCode(), TFT_BLACK);
    spr.drawString(w_stat, 125, 136);

    // Row 4: Signal & Free RAM
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    spr.drawString("SYSTEM:", 60, 150);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    String sys_str = (connected ? String(WiFi.RSSI()) + " dBm" : "AP") + " | RAM: " + String(ESP.getFreeHeap() / 1024) + " KB";
    spr.drawString(sys_str, 125, 150);

    // Divider before System Logs Area
    spr.drawFastHLine(50, 168, 260, palette.dark_graphite.getRawCode());
    spr.setTextColor(palette.yellow.getRawCode(), TFT_BLACK);
    spr.drawCenterString("SYSTEM LOGS (10 LINES BUFFER)", cx, 176);

    // Reserved 10-line console area
    spr.drawRect(48, 190, 264, 114, palette.dark_graphite.getRawCode());
    spr.setTextColor(palette.grey.getRawCode(), TFT_BLACK);
    
    spr.drawString("> [00:00:01] Display GC9B72 init OK", 54, 195);
    spr.drawString("> [00:00:01] 8-bit Sprite allocated", 54, 206);
    spr.drawString("> [00:00:02] DHT11 sensor ready", 54, 217);
    
    if (connected) {
        spr.setTextColor(palette.cyan.getRawCode(), TFT_BLACK);
        spr.drawString("> [NET] Wi-Fi connected, NTP active", 54, 228);
        spr.setTextColor(w_stat.indexOf("OK") >= 0 ? palette.lime.getRawCode() : palette.grapefruit.getRawCode(), TFT_BLACK);
        spr.drawString("> [API] " + w_stat, 54, 239);
    } else {
        spr.setTextColor(palette.grapefruit.getRawCode(), TFT_BLACK);
        spr.drawString("> [NET] Setup via 192.168.4.1", 54, 228);
    }
}