#ifndef WEB_STYLES_H
#define WEB_STYLES_H

#include <Arduino.h>

const char WEB_CSS[] PROGMEM = R"rawliteral(
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
  body { background-color: var(--bg); color: var(--text); display: flex; justify-content: center; padding: 20px; margin: 0; }
  .container { background: var(--card); border: 2px solid var(--border); border-radius: 24px; padding: 24px; width: 100%; max-width: 540px; }
  .header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
  .header h1 { font-size: 18px; letter-spacing: 2px; margin: 0; }
  .badge { font-size: 11px; color: var(--cyan); border: 1px solid var(--cyan); padding: 3px 10px; border-radius: 12px; }
  .tabs { display: flex; gap: 10px; margin-bottom: 20px; border-bottom: 1px solid var(--border); padding-bottom: 12px; }
  .tab-btn { flex: 1; background: var(--bg); color: var(--muted); border: 1px solid var(--border); padding: 10px; border-radius: 12px; cursor: pointer; font-weight: bold; font-size: 12px; }
  .tab-btn.active { color: var(--text); border-color: var(--cyan); background: rgba(0, 190, 255, 0.1); }
  .tab-content { display: none; }
  .tab-content.active { display: block; }
  .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 20px; }
  .metric { background: var(--bg); border: 1px solid var(--border); border-radius: 14px; padding: 12px; }
  .metric-label { font-size: 10px; color: var(--muted); text-transform: uppercase; letter-spacing: 1px; }
  .metric-val { font-size: 16px; font-weight: bold; margin-top: 4px; color: var(--cyan); }
  .metric-val.accent { color: var(--grapefruit); }
  .console { background: #050505; border: 1px solid var(--border); border-radius: 14px; padding: 12px; font-size: 12px; line-height: 1.6; height: 220px; overflow-y: auto; }
  .log-info { color: #A0A0A0; }
  .log-net { color: var(--cyan); }
  .log-err { color: var(--grapefruit); }
  .section-title { font-size: 12px; color: var(--cyan); letter-spacing: 1px; margin: 18px 0 10px 0; text-transform: uppercase; }
  .group { margin-bottom: 12px; }
  label.field-lbl { display: block; font-size: 11px; color: var(--muted); margin-bottom: 4px; }
  input[type="text"], input[type="password"] { width: 100%; background: var(--bg); border: 1px solid var(--border); color: var(--text); padding: 10px 12px; border-radius: 10px; font-size: 13px; outline: none; }
  
  /* Watch-style Graphite Pill Toggle Switch */
  .switch-row { display: flex; justify-content: space-between; align-items: center; background: var(--bg); border: 1px solid var(--border); padding: 10px 12px; border-radius: 12px; margin-bottom: 8px; }
  .switch-title { font-size: 13px; font-weight: bold; }
  .switch-sub { font-size: 11px; color: var(--muted); margin-top: 2px; }
  .pill-switch { position: relative; display: inline-block; width: 52px; height: 26px; flex-shrink: 0; margin-left: 12px; cursor: pointer; }
  .pill-switch input { opacity: 0; width: 0; height: 0; }
  .pill-slider { position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #262626; border-radius: 26px; transition: 0.2s; }
  .pill-slider:before { position: absolute; content: ""; height: 20px; width: 20px; left: 3px; bottom: 3px; background-color: #888888; border-radius: 50%; transition: 0.2s; }
  .pill-switch input:checked + .pill-slider { background-color: rgba(0, 190, 255, 0.25); border: 1px solid var(--cyan); }
  .pill-switch input:checked + .pill-slider:before { transform: translateX(25px); background-color: var(--cyan); }
  .pill-switch.accent input:checked + .pill-slider { background-color: rgba(252, 90, 44, 0.25); border: 1px solid var(--grapefruit); }
  .pill-switch.accent input:checked + .pill-slider:before { background-color: var(--grapefruit); }

  .btn { width: 100%; background: var(--grapefruit); color: var(--text); border: none; padding: 12px; border-radius: 12px; font-size: 12px; font-weight: bold; text-transform: uppercase; cursor: pointer; margin-top: 6px; }
  .btn-secondary { background: #1E1E1E; border: 1px solid var(--border); color: var(--cyan); }
  .wifi-item { display: flex; justify-content: space-between; align-items: center; background: var(--bg); border: 1px solid var(--border); padding: 8px 12px; border-radius: 10px; margin-bottom: 6px; font-size: 13px; }
  .del-btn { background: none; border: none; color: var(--grapefruit); cursor: pointer; font-weight: bold; }
  .status-msg { text-align: center; font-size: 12px; color: var(--lime); margin-top: 12px; min-height: 16px; }
)rawliteral";

#endif // WEB_STYLES_H