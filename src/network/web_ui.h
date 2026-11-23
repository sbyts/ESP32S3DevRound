#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>
#include "web_styles.h"
#include "web_script.h"

const char WEB_HTML_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Climate Core OS</title>
<style>
)rawliteral";

const char WEB_HTML_BODY[] PROGMEM = R"rawliteral(
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h1>CLIMATE CORE OS</h1>
    <span class="badge" id="ip_badge">ONLINE</span>
  </div>

  <div class="tabs">
    <button type="button" class="tab-btn active" id="btn_sys" onclick="openTab('tab_sys', 'btn_sys')">SYSTEM & LOGS</button>
    <button type="button" class="tab-btn" id="btn_cfg" onclick="openTab('tab_cfg', 'btn_cfg')">CONFIG & WI-FI</button>
  </div>

  <div id="tab_sys" class="tab-content active">
    <div class="grid">
      <div class="metric"><div class="metric-label">Uptime</div><div class="metric-val" id="m_uptime">--</div></div>
      <div class="metric"><div class="metric-label">Chip Temp / CPU</div><div class="metric-val accent" id="m_cpu">--</div></div>
      <div class="metric"><div class="metric-label">Room Climate / Light</div><div class="metric-val" id="m_env">--</div></div>
      <div class="metric"><div class="metric-label">Free Heap RAM</div><div class="metric-val" id="m_free_ram">--</div></div>
      <div class="metric"><div class="metric-label">Used / Total RAM</div><div class="metric-val" id="m_used_ram">--</div></div>
      <div class="metric"><div class="metric-label">Min Free Heap</div><div class="metric-val" id="m_min_ram">--</div></div>
      <div class="metric"><div class="metric-label">Wi-Fi Signal / Boots</div><div class="metric-val" id="m_net">--</div></div>
      <div class="metric"><div class="metric-label">I2C Sensors (SHT / BH)</div><div class="metric-val" id="m_i2c">--</div></div>
    </div>

    <div class="section-title">Ring Buffer Logs (Last 10 Events)</div>
    <div class="console" id="log_box">Loading logs...</div>
    <button type="button" class="btn btn-secondary" onclick="loadTelemetry(null)">Refresh Telemetry & Logs</button>
  </div>

  <div id="tab_cfg" class="tab-content">
    <div class="section-title">Saved Wi-Fi Networks (Max 6)</div>
    <div id="wifi_list"></div>
    
    <div style="display: flex; gap: 8px; margin-top: 8px;">
      <input type="text" id="new_ssid" placeholder="New Wi-Fi SSID">
      <input type="password" id="new_pass" placeholder="Password">
    </div>
    <button type="button" class="btn btn-secondary" onclick="addWifi()">+ Add Wi-Fi Network</button>

    <div class="section-title">Battery & Presence Power Saving</div>
    <div class="switch-row">
      <div>
        <div class="switch-title">HLK-LD2410 Presence Radar</div>
        <div class="switch-sub">Auto-sleep display when room is empty</div>
      </div>
      <label class="pill-switch">
        <input type="checkbox" id="cfg_radar_en">
        <span class="pill-slider"></span>
      </label>
    </div>
    <div class="switch-row">
      <div>
        <div class="switch-title">Deep Wi-Fi Off in Eco Mode</div>
        <div class="switch-sub">OFF = Modem Sleep | ON = Radio OFF</div>
      </div>
      <label class="pill-switch accent">
        <input type="checkbox" id="cfg_eco_wifi">
        <span class="pill-slider"></span>
      </label>
    </div>
    <div class="group">
      <label class="field-lbl">Display Sleep Delay after Empty Room (sec)</label>
      <input type="number" id="cfg_screen_delay" min="5" max="3600" placeholder="15">
    </div>

    <div class="section-title">Ridy-10-Lite Lamp Link &amp; Lux Trigger</div>
    <div class="switch-row">
      <div>
        <div class="switch-title">Auto Presence Lamp Control</div>
        <div class="switch-sub">Turn lamp ON on arrival &amp; delayed OFF when away</div>
      </div>
      <label class="pill-switch">
        <input type="checkbox" id="cfg_lamp_auto">
        <span class="pill-slider"></span>
      </label>
    </div>
    <div class="group">
      <label class="field-lbl">Ambient Light Threshold for Lamp Trigger (lux)</label>
      <input type="number" id="cfg_lux_thresh" min="0" max="10000" placeholder="40">
    </div>
    <div class="group">
      <label class="field-lbl">Lamp Auto-Off Delay (sec)</label>
      <input type="number" id="cfg_lamp_delay" min="0" max="3600" placeholder="10">
    </div>
    <div class="group">
      <label class="field-lbl">Lamp URL / IP</label>
      <input type="text" id="cfg_lamp_url" placeholder="http://192.168.0.13">
    </div>
    <div class="group">
      <label class="field-lbl">Lamp Link API Key</label>
      <input type="password" id="cfg_lamp_key" placeholder="ridy-desk-2026">
    </div>

    <div class="section-title">Weather & Security</div>
    <div class="group">
      <label class="field-lbl">Weather Location (City in English)</label>
      <input type="text" id="cfg_city" placeholder="Kyiv">
    </div>
    <div class="group">
      <label class="field-lbl">New Web Admin Password (Login: admin)</label>
      <input type="password" id="cfg_admin_pass" placeholder="Leave blank to keep current">
    </div>

    <button type="button" class="btn" onclick="saveSettings()">Save All & Reboot</button>
    <div class="status-msg" id="save_msg"></div>
  </div>
</div>
<script>
)rawliteral";

const char WEB_HTML_FOOT[] PROGMEM = R"rawliteral(
</script>
</body>
</html>
)rawliteral";

#endif // WEB_UI_H