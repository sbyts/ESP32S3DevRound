#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Climate Core OS</title>
<style>
  :root {
    --bg: #000000;
    --card: #111111;
    --border: #262626;
    --grapefruit: #FC5A2C;
    --cyan: #00BEFF;
    --lime: #32D74B;
    --text: #FFFFFF;
    --muted: #757575;
  }
  * { box-sizing: border-box; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, monospace; }
  body {
    background-color: var(--bg);
    color: var(--text);
    display: flex;
    justify-content: center;
    padding: 20px;
    margin: 0;
  }
  .container {
    background: var(--card);
    border: 2px solid var(--border);
    border-radius: 24px;
    padding: 24px;
    width: 100%;
    max-width: 540px;
    box-shadow: 0 0 40px rgba(0, 190, 255, 0.08);
  }
  .header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
  .header h1 { font-size: 18px; letter-spacing: 2px; margin: 0; }
  .badge { font-size: 11px; color: var(--cyan); border: 1px solid var(--cyan); padding: 3px 10px; border-radius: 12px; }
  
  /* Tabs */
  .tabs { display: flex; gap: 10px; margin-bottom: 20px; border-bottom: 1px solid var(--border); padding-bottom: 12px; }
  .tab-btn {
    flex: 1; background: var(--bg); color: var(--muted); border: 1px solid var(--border);
    padding: 10px; border-radius: 12px; cursor: pointer; font-weight: bold; font-size: 12px; letter-spacing: 1px;
  }
  .tab-btn.active { color: var(--text); border-color: var(--cyan); background: rgba(0, 190, 255, 0.1); }
  .tab-content { display: none; }
  .tab-content.active { display: block; }

  /* Telemetry Grid */
  .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 20px; }
  .metric { background: var(--bg); border: 1px solid var(--border); border-radius: 14px; padding: 12px; }
  .metric-label { font-size: 10px; color: var(--muted); text-transform: uppercase; letter-spacing: 1px; }
  .metric-val { font-size: 16px; font-weight: bold; margin-top: 4px; color: var(--cyan); }
  .metric-val.accent { color: var(--grapefruit); }

  /* Console */
  .console {
    background: #050505; border: 1px solid var(--border); border-radius: 14px;
    padding: 12px; font-size: 12px; line-height: 1.6; height: 230px; overflow-y: auto;
  }
  .log-info { color: #A0A0A0; }
  .log-net  { color: var(--cyan); }
  .log-err  { color: var(--grapefruit); }

  /* Forms & Wi-Fi List */
  .section-title { font-size: 12px; color: var(--cyan); letter-spacing: 1px; margin: 18px 0 10px 0; text-transform: uppercase; }
  .group { margin-bottom: 12px; }
  label { display: block; font-size: 11px; color: var(--muted); margin-bottom: 4px; }
  input {
    width: 100%; background: var(--bg); border: 1px solid var(--border); color: var(--text);
    padding: 10px 12px; border-radius: 10px; font-size: 13px; outline: none;
  }
  input:focus { border-color: var(--cyan); }
  .btn {
    width: 100%; background: var(--grapefruit); color: var(--text); border: none;
    padding: 12px; border-radius: 12px; font-size: 12px; font-weight: bold; letter-spacing: 1px;
    text-transform: uppercase; cursor: pointer; margin-top: 6px;
  }
  .btn-secondary { background: #1E1E1E; border: 1px solid var(--border); color: var(--cyan); }
  
  .wifi-item {
    display: flex; justify-content: space-between; align-items: center;
    background: var(--bg); border: 1px solid var(--border); padding: 8px 12px;
    border-radius: 10px; margin-bottom: 6px; font-size: 13px;
  }
  .del-btn { background: none; border: none; color: var(--grapefruit); cursor: pointer; font-weight: bold; }
  .status-msg { text-align: center; font-size: 12px; color: var(--lime); margin-top: 12px; min-height: 16px; }
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h1>CLIMATE CORE OS</h1>
    <span class="badge" id="ip_badge">CONNECTING...</span>
  </div>

  <div class="tabs">
    <button class="tab-btn active" onclick="switchTab('tab_sys', this)">SYSTEM & LOGS</button>
    <button class="tab-btn" onclick="switchTab('tab_cfg', this)">CONFIG & WI-FI</button>
  </div>

  <!-- TAB 1: SYSTEM TELEMETRY & LIVE LOGS -->
  <div id="tab_sys" class="tab-content active">
    <div class="grid">
      <div class="metric"><div class="metric-label">Uptime</div><div class="metric-val" id="m_uptime">--</div></div>
      <div class="metric"><div class="metric-label">Chip Temp / CPU</div><div class="metric-val accent" id="m_cpu">--</div></div>
      <div class="metric"><div class="metric-label">Free Heap RAM</div><div class="metric-val" id="m_free_ram">--</div></div>
      <div class="metric"><div class="metric-label">Used / Total RAM</div><div class="metric-val" id="m_used_ram">--</div></div>
      <div class="metric"><div class="metric-label">Min Free Heap</div><div class="metric-val" id="m_min_ram">--</div></div>
      <div class="metric"><div class="metric-label">Wi-Fi Signal / Boots</div><div class="metric-val" id="m_net">--</div></div>
    </div>

    <div class="section-title">Ring Buffer Logs (Last 10 Events)</div>
    <div class="console" id="log_box">Loading logs...</div>
    <button class="btn btn-secondary" onclick="loadTelemetry()">Refresh Telemetry & Logs</button>
  </div>

  <!-- TAB 2: CONFIGURATION, 6 WI-FI SLOTS & AUTH -->
  <div id="tab_cfg" class="tab-content">
    <div class="section-title">Saved Wi-Fi Networks (Max 6)</div>
    <div id="wifi_list"></div>
    
    <div style="display: flex; gap: 8px; margin-top: 8px;">
      <input type="text" id="new_ssid" placeholder="New Wi-Fi SSID">
      <input type="password" id="new_pass" placeholder="Password">
    </div>
    <button class="btn btn-secondary" onclick="addWifi()">+ Add Wi-Fi Network</button>

    <div class="section-title">Weather & Security</div>
    <div class="group">
      <label>Weather Location (City in English)</label>
      <input type="text" id="cfg_city" placeholder="Kyiv">
    </div>
    <div class="group">
      <label>New Web Admin Password (Login: admin)</label>
      <input type="password" id="cfg_admin_pass" placeholder="Leave blank to keep current">
    </div>

    <button class="btn" onclick="saveSettings()">Save All & Reboot</button>
    <div class="status-msg" id="save_msg"></div>
  </div>
</div>

<script>
  let savedNetworks = [];

  function switchTab(tabId, btn) {
    document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.tab-btn').forEach(el => el.classList.remove('active'));
    document.getElementById(tabId).classList.add('active');
    btn.classList.add('active');
  }

  function loadTelemetry() {
    fetch('/api/telemetry').then(r => r.json()).then(d => {
      document.getElementById('ip_badge').innerText = d.ip;
      document.getElementById('m_uptime').innerText = d.uptime;
      document.getElementById('m_cpu').innerText = d.temp + '°C / ' + d.cpu + ' MHz';
      document.getElementById('m_free_ram').innerText = d.free_kb + ' KB';
      document.getElementById('m_used_ram').innerText = d.used_kb + ' / ' + d.total_kb + ' KB';
      document.getElementById('m_min_ram').innerText = d.min_kb + ' KB';
      document.getElementById('m_net').innerText = d.rssi + ' dBm | #' + d.boots;

      const box = document.getElementById('log_box');
      box.innerHTML = '';
      d.logs.forEach(item => {
        const div = document.createElement('div');
        div.className = item.lvl === 1 ? 'log-net' : (item.lvl === 2 ? 'log-err' : 'log-info');
        div.innerText = item.txt;
        box.appendChild(div);
      });
    });
  }

  function renderWifiList() {
    const list = document.getElementById('wifi_list');
    list.innerHTML = '';
    if (savedNetworks.length === 0) {
      list.innerHTML = '<div style="color:#757575; font-size:12px;">No saved networks yet.</div>';
      return;
    }
    savedNetworks.forEach((net, idx) => {
      const row = document.createElement('div');
      row.className = 'wifi-item';
      row.innerHTML = `<span><b>${idx + 1}.</b> ${net.ssid}</span>
                       <button class="del-btn" onclick="removeWifi(${idx})">✕</button>`;
      list.appendChild(row);
    });
  }

  function addWifi() {
    const s = document.getElementById('new_ssid').value.trim();
    const p = document.getElementById('new_pass').value;
    if (!s) return;
    if (savedNetworks.length >= 6) {
      alert('Maximum 6 Wi-Fi networks allowed! Delete one first.');
      return;
    }
    savedNetworks.push({ ssid: s, pass: p });
    document.getElementById('new_ssid').value = '';
    document.getElementById('new_pass').value = '';
    renderWifiList();
  }

  function removeWifi(idx) {
    savedNetworks.splice(idx, 1);
    renderWifiList();
  }

  function loadConfig() {
    fetch('/api/config').then(r => r.json()).then(d => {
      document.getElementById('cfg_city').value = d.city || 'Kyiv';
      savedNetworks = d.networks || [];
      renderWifiList();
    });
  }

  function saveSettings() {
    const payload = {
      city: document.getElementById('cfg_city').value.trim(),
      admin_pass: document.getElementById('cfg_admin_pass').value,
      networks: savedNetworks
    };
    document.getElementById('save_msg').innerText = 'SAVING CONFIGURATION...';
    fetch('/api/save', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    }).then(() => {
      document.getElementById('save_msg').innerText = 'SAVED! REBOOTING DEVICE...';
    });
  }

  loadTelemetry();
  loadConfig();
  setInterval(loadTelemetry, 5000);
</script>
</body>
</html>
)rawliteral";

#endif // WEB_UI_H