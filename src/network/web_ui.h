#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Climate Station Config</title>
<style>
  :root {
    --bg: #000000;
    --card: #121212;
    --border: #262626;
    --grapefruit: #FC5A2C;
    --cyan: #00BEFF;
    --text: #FFFFFF;
    --muted: #707070;
  }
  * { box-sizing: border-box; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
  body {
    background-color: var(--bg);
    color: var(--text);
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
    margin: 0;
    padding: 20px;
  }
  .dial-card {
    background: var(--card);
    border: 2px solid var(--border);
    border-radius: 28px;
    padding: 32px 28px;
    width: 100%;
    max-width: 380px;
    box-shadow: 0 0 40px rgba(0, 190, 255, 0.08);
  }
  .header {
    text-align: center;
    margin-bottom: 28px;
  }
  .header h1 {
    font-size: 20px;
    letter-spacing: 2px;
    margin: 0 0 6px 0;
    text-transform: uppercase;
  }
  .badge {
    display: inline-block;
    font-size: 11px;
    color: var(--cyan);
    border: 1px solid var(--cyan);
    padding: 2px 10px;
    border-radius: 12px;
    letter-spacing: 1px;
  }
  .group { margin-bottom: 18px; }
  label {
    display: block;
    font-size: 11px;
    color: var(--muted);
    text-transform: uppercase;
    letter-spacing: 1px;
    margin-bottom: 6px;
  }
  input {
    width: 100%;
    background: var(--bg);
    border: 1px solid var(--border);
    color: var(--text);
    padding: 12px 14px;
    border-radius: 12px;
    font-size: 15px;
    outline: none;
    transition: border-color 0.2s;
  }
  input:focus { border-color: var(--cyan); }
  .btn {
    width: 100%;
    background: var(--grapefruit);
    color: var(--text);
    border: none;
    padding: 14px;
    border-radius: 14px;
    font-size: 14px;
    font-weight: 700;
    letter-spacing: 1.5px;
    text-transform: uppercase;
    cursor: pointer;
    margin-top: 10px;
    transition: opacity 0.2s;
  }
  .btn:hover { opacity: 0.9; }
  .status {
    text-align: center;
    font-size: 12px;
    color: var(--cyan);
    margin-top: 16px;
    min-height: 18px;
  }
</style>
</head>
<body>
  <div class="dial-card">
    <div class="header">
      <h1>Climate Core</h1>
      <span class="badge" id="net_status">SYSTEM ONLINE</span>
    </div>
    <form id="cfgForm">
      <div class="group">
        <label>Wi-Fi Network (SSID)</label>
        <input type="text" id="ssid" name="ssid" placeholder="Enter Wi-Fi name" required>
      </div>
      <div class="group">
        <label>Wi-Fi Password</label>
        <input type="password" id="pass" name="pass" placeholder="••••••••">
      </div>
      <div class="group">
        <label>Weather Location (City in English)</label>
        <input type="text" id="city" name="city" placeholder="e.g. Kyiv, Lviv, London" required>
      </div>
      <button type="submit" class="btn">Save & Reboot</button>
    </form>
    <div class="status" id="msg"></div>
  </div>

<script>
  fetch('/api/config').then(r => r.json()).then(d => {
    if(d.ssid) document.getElementById('ssid').value = d.ssid;
    if(d.city) document.getElementById('city').value = d.city;
    if(d.ip) document.getElementById('net_status').innerText = 'IP: ' + d.ip;
  });

  document.getElementById('cfgForm').onsubmit = function(e) {
    e.preventDefault();
    const formData = new FormData(this);
    document.getElementById('msg').innerText = 'SAVING SETTINGS...';
    fetch('/api/save', { method: 'POST', body: formData })
      .then(() => {
        document.getElementById('msg').innerText = 'SAVED! REBOOTING DEVICE...';
      });
  };
</script>
</body>
</html>
)rawliteral";

#endif // WEB_UI_H