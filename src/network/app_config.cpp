#include "app_config.h"

AppConfig appConfig;

void AppConfig::load()
{
    _prefs.begin("climate_cfg", false);
    screenDelaySec = _prefs.getUInt("scr_delay", DEFAULT_SCREEN_DELAY); 
    bootCount = _prefs.getUInt("boots", 0) + 1;
    _prefs.putUInt("boots", bootCount);

    city = _prefs.getString("city", "Kyiv");
    adminPass = _prefs.getString("admin_pass", "admin");

    // Disabled by default so the screen stays ON until enabled via Web UI
    radarEnabled = _prefs.getBool("radar_en", false);
    ecoWifiOff = _prefs.getBool("eco_wifi", false);

    // Load Ridy-10-Lite Lamp Link & Lux settings with config.h defaults
    lampUrl = _prefs.getString("lamp_url", DEFAULT_LAMP_URL);
    lampKey = _prefs.getString("lamp_key", DEFAULT_LAMP_KEY);
    lampDelaySec = _prefs.getUInt("lamp_delay", DEFAULT_LAMP_DELAY);
    lampAutoPresence = _prefs.getBool("lamp_auto", true);
    luxThreshold = _prefs.getUInt("lux_thresh", DEFAULT_LUX_THRESHOLD);

    netCount = 0;
    for (uint8_t i = 0; i < MAX_WIFI_NETWORKS; i++)
    {
        String s_key = "ssid_" + String(i);
        String p_key = "pass_" + String(i);
        String s_val = _prefs.getString(s_key.c_str(), "");
        if (s_val.length() > 0)
        {
            networks[netCount].ssid = s_val;
            networks[netCount].pass = _prefs.getString(p_key.c_str(), "");
            netCount++;
        }
    }

    // Migrate legacy single-SSID setting safely
    if (netCount == 0)
    {
        String legacy_ssid = _prefs.getString("ssid", "");
        if (legacy_ssid.length() > 0)
        {
            networks[0].ssid = legacy_ssid;
            networks[0].pass = _prefs.getString("pass", "");
            netCount = 1;
            saveWiFiNetworks();
        }
    }
}

void AppConfig::saveWiFiNetworks()
{
    for (uint8_t i = 0; i < MAX_WIFI_NETWORKS; i++)
    {
        String s_key = "ssid_" + String(i);
        String p_key = "pass_" + String(i);
        if (i < netCount)
        {
            _prefs.putString(s_key.c_str(), networks[i].ssid);
            _prefs.putString(p_key.c_str(), networks[i].pass);
        }
        else
        {
            _prefs.remove(s_key.c_str());
            _prefs.remove(p_key.c_str());
        }
    }
}

void AppConfig::setCity(const String& newCity)
{
    city = newCity;
    _prefs.putString("city", city);
}

void AppConfig::setAdminPass(const String& newPass)
{
    adminPass = newPass;
    _prefs.putString("admin_pass", adminPass);
}

void AppConfig::setRadarEnabled(bool enabled)
{
    radarEnabled = enabled;
    _prefs.putBool("radar_en", radarEnabled);
}

void AppConfig::setEcoWifiOff(bool enabled)
{
    ecoWifiOff = enabled;
    _prefs.putBool("eco_wifi", ecoWifiOff);
}

void AppConfig::setLampUrl(const String& url)
{
    lampUrl = url;
    _prefs.putString("lamp_url", lampUrl);
}

void AppConfig::setLampKey(const String& key)
{
    lampKey = key;
    _prefs.putString("lamp_key", lampKey);
}

void AppConfig::setLampDelay(uint32_t delaySec)
{
    lampDelaySec = delaySec;
    _prefs.putUInt("lamp_delay", lampDelaySec);
}

void AppConfig::setLampAuto(bool autoEn)
{
    lampAutoPresence = autoEn;
    _prefs.putBool("lamp_auto", lampAutoPresence);
}

void AppConfig::setScreenDelay(uint32_t delaySec)
{
    screenDelaySec = delaySec;
    _prefs.putUInt("scr_delay", screenDelaySec);
}

void AppConfig::setLuxThreshold(uint32_t lux)
{
    luxThreshold = lux;
    _prefs.putUInt("lux_thresh", luxThreshold);
}