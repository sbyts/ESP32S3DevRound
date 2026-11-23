#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include "../config/config.h"

struct WifiCreds {
    String ssid;
    String pass;
};

class AppConfig {
public:
    static const uint8_t MAX_WIFI_NETWORKS = 6;
    uint32_t screenDelaySec = DEFAULT_SCREEN_DELAY;
    // General & security settings
    String city = "Kyiv";
    String adminPass = "admin";
    uint32_t bootCount = 0;

    // Power & radar settings
    bool radarEnabled = false;
    bool ecoWifiOff = false;

    // Ridy-10-Lite Lamp Link settings
    String lampUrl = DEFAULT_LAMP_URL;
    String lampKey = DEFAULT_LAMP_KEY;
    uint32_t lampDelaySec = DEFAULT_LAMP_DELAY;
    bool lampAutoPresence = true;

    // Multi-AP Wi-Fi credentials
    WifiCreds networks[MAX_WIFI_NETWORKS];
    uint8_t netCount = 0;

    void load();
    void saveWiFiNetworks();

    // Individual setters with immediate NVS persistence
    void setCity(const String& newCity);
    void setAdminPass(const String& newPass);
    void setRadarEnabled(bool enabled);
    void setEcoWifiOff(bool enabled);
    void setLampUrl(const String& url);
    void setLampKey(const String& key);
    void setLampDelay(uint32_t delaySec);
    void setLampAuto(bool autoEn);
    void setScreenDelay(uint32_t delaySec);

private:
    Preferences _prefs;
};

extern AppConfig appConfig;

#endif // APP_CONFIG_H