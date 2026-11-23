#ifndef WEB_SCRIPT_H
#define WEB_SCRIPT_H

#include <Arduino.h>

const char WEB_JS[] PROGMEM = R"rawliteral(
  var savedNetworks = [];

  function openTab(tabId, btnId) {
    document.getElementById('tab_sys').className = 'tab-content';
    document.getElementById('tab_cfg').className = 'tab-content';
    document.getElementById('btn_sys').className = 'tab-btn';
    document.getElementById('btn_cfg').className = 'tab-btn';
    document.getElementById(tabId).className = 'tab-content active';
    document.getElementById(btnId).className = 'tab-btn active';
  }

  function loadTelemetry(callback) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', '/api/telemetry?t=' + Date.now(), true);
    xhr.onreadystatechange = function() {
      if (xhr.readyState === 4) {
        if (xhr.status === 200) {
          try {
            var d = JSON.parse(xhr.responseText);
            document.getElementById('ip_badge').innerText = d.ip || 'ONLINE';
            document.getElementById('m_uptime').innerText = d.uptime || '--';
            document.getElementById('m_cpu').innerText = d.temp + ' C / ' + d.cpu + ' MHz';
            document.getElementById('m_free_ram').innerText = d.free_kb + ' KB';
            document.getElementById('m_used_ram').innerText = d.used_kb + ' / ' + d.total_kb + ' KB';
            document.getElementById('m_min_ram').innerText = d.min_kb + ' KB';
            document.getElementById('m_net').innerText = d.rssi + ' dBm | #' + d.boots;

            var box = document.getElementById('log_box');
            box.innerHTML = '';
            if (d.logs && d.logs.length > 0) {
              for (var i = 0; i < d.logs.length; i++) {
                var item = d.logs[i];
                var div = document.createElement('div');
                div.className = item.lvl === 1 ? 'log-net' : (item.lvl === 2 ? 'log-err' : 'log-info');
                div.innerText = item.txt;
                box.appendChild(div);
              }
            } else {
              box.innerText = 'No logs yet.';
            }
          } catch (e) {
            document.getElementById('log_box').innerText = 'JSON Error: ' + e.message;
          }
        } else {
          document.getElementById('log_box').innerText = 'HTTP Status: ' + xhr.status;
        }
        if (callback) callback();
      }
    };
    xhr.send();
  }

  function renderWifiList() {
    var list = document.getElementById('wifi_list');
    list.innerHTML = '';
    if (savedNetworks.length === 0) {
      list.innerHTML = '<div style="color:#757575; font-size:12px;">No saved networks yet.</div>';
      return;
    }
    for (var i = 0; i < savedNetworks.length; i++) {
      var net = savedNetworks[i];
      var row = document.createElement('div');
      row.className = 'wifi-item';
      row.innerHTML = '<span><b>' + (i + 1) + '.</b> ' + net.ssid + '</span>' +
                      '<button type="button" class="del-btn" onclick="removeWifi(' + i + ')">X</button>';
      list.appendChild(row);
    }
  }

  function addWifi() {
    var s = document.getElementById('new_ssid').value.trim();
    var p = document.getElementById('new_pass').value;
    if (!s) return;
    if (savedNetworks.length >= 6) {
      alert('Maximum 6 Wi-Fi networks allowed!');
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
    var xhr = new XMLHttpRequest();
    xhr.open('GET', '/api/config?t=' + Date.now(), true);
    xhr.onreadystatechange = function() {
      if (xhr.readyState === 4 && xhr.status === 200) {
        try {
          var d = JSON.parse(xhr.responseText);
          document.getElementById('cfg_city').value = d.city || 'Kyiv';
          document.getElementById('cfg_radar_en').checked = (d.radar_en === true);
          document.getElementById('cfg_eco_wifi').checked = (d.eco_wifi === true);
          document.getElementById('cfg_screen_delay').value = d.screen_delay;
          document.getElementById('cfg_lamp_delay').value = d.lamp_delay;
          document.getElementById('cfg_lamp_auto').checked = (d.lamp_auto === true);
          document.getElementById('cfg_lamp_url').value = d.lamp_url || '';
          document.getElementById('cfg_lamp_key').value = d.lamp_key || '';
          savedNetworks = d.networks || [];
          renderWifiList();
        } catch (e) {}
      }
    };
    xhr.send();
  }

  function saveSettings() {
    var payload = {
      city: document.getElementById('cfg_city').value.trim(),
      admin_pass: document.getElementById('cfg_admin_pass').value,
      radar_en: document.getElementById('cfg_radar_en').checked,
      eco_wifi: document.getElementById('cfg_eco_wifi').checked,
      screen_delay: parseInt(document.getElementById('cfg_screen_delay').value),
      lamp_delay: parseInt(document.getElementById('cfg_lamp_delay').value),
      lamp_auto: document.getElementById('cfg_lamp_auto').checked,
      lamp_url: document.getElementById('cfg_lamp_url').value.trim(),
      lamp_key: document.getElementById('cfg_lamp_key').value.trim(),
      networks: savedNetworks
    };
    document.getElementById('save_msg').innerText = 'SAVING CONFIGURATION...';
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/api/save', true);
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.onreadystatechange = function() {
      if (xhr.readyState === 4) {
        document.getElementById('save_msg').innerText = 'SAVED! REBOOTING...';
      }
    };
    xhr.send(JSON.stringify(payload));
  }

  window.onload = function() {
    loadTelemetry(function() {
      loadConfig();
    });
    setInterval(function() { loadTelemetry(null); }, 5000);
  };
)rawliteral";

#endif // WEB_SCRIPT_H