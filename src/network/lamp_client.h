#ifndef LAMP_CLIENT_H
#define LAMP_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "../config/config.h"
#include "app_config.h"
#include "../peripherals/radar_ld2410.h"
#include "../peripherals/sensor_climate.h"
#include "../system/sys_logger.h"

class LampClient {
public:
    // Live lamp status (read safely by UI on Core 1)
    volatile bool online = false;
    volatile bool isOn = false;
    volatile uint8_t mode = 1;      // 1: 3000K, 2: 6000K, 3: 4500K
    volatile int brightness = 80;   // 10..100%

    // Presence automation settings
    volatile bool autoPresence = true;
    volatile uint32_t offDelaySec = DEFAULT_LAMP_DELAY; // Default 10s
    volatile bool userPresent = false;
    volatile uint32_t awayElapsedSec = 0;

    void begin(const String& baseUrl = DEFAULT_LAMP_URL,
               const String& linkKey = DEFAULT_LAMP_KEY,
               uint32_t delaySec = DEFAULT_LAMP_DELAY) {
        _baseUrl = baseUrl;
        _baseUrl.trim();
        if (_baseUrl.endsWith("/")) {
            _baseUrl.remove(_baseUrl.length() - 1);
        }
        if (_baseUrl.endsWith("/api/link")) {
            _baseUrl.remove(_baseUrl.length() - 9);
        }

        _key = linkKey;
        offDelaySec = delaySec;
        _cachedResolvedBase = "";
    }

    // Called from Core 1 (loop): evaluates presence & queues immediate delayed-OFF command
    void updatePresence(bool rawPinHigh) {
        if (!autoPresence || !radar.isEnabled()) {
            userPresent = false;
            awayElapsedSec = 0;
            _awayStartMs = 0;
            _sentAwayCmdForCurrentCycle = false;
            return;
        }

        unsigned long now = millis();

        if (rawPinHigh) {
            userPresent = true;
            _lowDebounceStartMs = 0;
            _awayStartMs = 0;
            awayElapsedSec = 0;
            _sentAwayCmdForCurrentCycle = false;

            // Daylight-aware check: only turn ON if room is darker than threshold (or if BH1750 is absent)
            bool is_dark_enough = !climateSensor.isBhReady() ||
                                  (climateSensor.getLux() <= (float)appConfig.luxThreshold);

            if (!isOn && is_dark_enough && _pendingCmd != 1 && (now - _lastCmdAttemptMs > 1500UL)) {
                _lastCmdAttemptMs = now;
                _pendingCmd = 1;
            }
        } else {
            // Short 400ms debounce so brief LD2410 micro-drops don't spam HTTP requests
            if (_lowDebounceStartMs == 0) {
                _lowDebounceStartMs = now;
            }
            if (now - _lowDebounceStartMs < 400UL) {
                return;
            }

            userPresent = false;

            if (_awayStartMs == 0) {
                _awayStartMs = now;
            }

            // Local countdown progress for smooth UI arc rendering before screen sleeps
            uint32_t elapsed = (now - _awayStartMs) / 1000UL;
            awayElapsedSec = (elapsed <= offDelaySec) ? elapsed : offDelaySec;

            // Send immediate POST ?presence=0&delay=<offDelaySec> once when user leaves
            if (isOn && !_sentAwayCmdForCurrentCycle && _pendingCmd != 0 && (now - _lastCmdAttemptMs > 1500UL)) {
                _lastCmdAttemptMs = now;
                _pendingCmd = 0;
            }
        }
    }

    // Called exclusively from Core 0 (NetWorkerTask)
    void process() {
        if (WiFi.status() != WL_CONNECTED) {
            online = false;
            _cachedResolvedBase = "";
            return;
        }

        // 1. Handle queued presence command immediately
        int8_t cmd = _pendingCmd;
        if (cmd != -1) {
            _pendingCmd = -1;
            bool turnOn = (cmd == 1);
            String cmdUrl = resolveEndpointUrl() + "?presence=" + (turnOn ? "1" : "0");
            if (!turnOn) {
                cmdUrl += "&delay=" + String((uint32_t)offDelaySec);
            }

            if (executeHttp("POST", cmdUrl, true)) {
                _ignorePollUntilMs = millis() + 1200UL;
                if (turnOn) {
                    isOn = true;
                    _sentAwayCmdForCurrentCycle = false;
                    sys_log.log("Lamp Link -> Presence ON", LOG_INFO);
                } else {
                    _sentAwayCmdForCurrentCycle = true;
                    sys_log.log("Lamp Link -> Sent OFF in " + String((uint32_t)offDelaySec) + "s", LOG_INFO);
                }
            }
            _lastPollMs = millis();
            return;
        }

        // 2. Periodic status poll: 3s when online, 12s backoff when offline
        if (millis() < _ignorePollUntilMs) return;

        unsigned long interval = online ? 3000UL : 12000UL;
        if (millis() - _lastPollMs >= interval) {
            _lastPollMs = millis();
            executeHttp("GET", resolveEndpointUrl(), false);
        }
    }

    // Legacy alias
    void pollStatus() { process(); }

private:
    String _baseUrl = DEFAULT_LAMP_URL;
    String _key = DEFAULT_LAMP_KEY;
    String _cachedResolvedBase = "";
    unsigned long _lastMdnsResolveMs = 0;

    volatile bool _sentAwayCmdForCurrentCycle = false;
    volatile int8_t _pendingCmd = -1; // -1: none, 0: send delayed OFF, 1: send immediate ON
    unsigned long _lowDebounceStartMs = 0;
    unsigned long _awayStartMs = 0;
    unsigned long _lastCmdAttemptMs = 0;
    unsigned long _lastPollMs = 0;
    unsigned long _ignorePollUntilMs = 0;

    String resolveEndpointUrl() {
        if (_cachedResolvedBase.length() > 0 && (online || (millis() - _lastMdnsResolveMs < 30000UL))) {
            return _cachedResolvedBase + "/api/link";
        }

        String resolvedBase = _baseUrl;
        int protoIdx = resolvedBase.indexOf("://");
        if (protoIdx != -1) {
            String host = resolvedBase.substring(protoIdx + 3);
            int portIdx = host.indexOf(':');
            String portPart = "";
            if (portIdx != -1) {
                portPart = host.substring(portIdx);
                host = host.substring(0, portIdx);
            }

            if (host.endsWith(".local")) {
                _lastMdnsResolveMs = millis();
                String mdnsName = host.substring(0, host.length() - 6);
                IPAddress ip = MDNS.queryHost(mdnsName.c_str(), 150);
                if (ip != IPAddress(0, 0, 0, 0)) {
                    resolvedBase = "http://" + ip.toString() + portPart;
                    _cachedResolvedBase = resolvedBase;
                } else {
                    _cachedResolvedBase = "";
                }
            } else {
                _cachedResolvedBase = resolvedBase;
            }
        }
        return resolvedBase + "/api/link";
    }

    bool executeHttp(const char* method, const String& targetUrl, bool isCommand) {
        WiFiClient client;
        HTTPClient http;
        http.setConnectTimeout(200);
        http.setTimeout(250);
        http.setReuse(false);

        if (!http.begin(client, targetUrl)) {
            online = false;
            return false;
        }

        http.addHeader("X-Link-Key", _key);
        int code = (strcmp(method, "POST") == 0) ? http.POST("") : http.GET();

        if (code == 200) {
            String payload = http.getString();
            online = true;

            if (payload.indexOf("\"on\":") != -1) {
                isOn = (payload.indexOf("\"on\":true") != -1);
            }

            int mIdx = payload.indexOf("\"mode\":");
            if (mIdx != -1) mode = payload.substring(mIdx + 7).toInt();

            int bIdx = payload.indexOf("\"bri\":");
            if (bIdx != -1) brightness = payload.substring(bIdx + 6).toInt();

            // Sync countdown arc with lamp's actual remaining seconds if timer is running
            int offIdx = payload.indexOf("\"off_in\":");
            if (offIdx != -1 && !userPresent && isOn) {
                uint32_t remSec = payload.substring(offIdx + 9).toInt();
                if (remSec > 0 && offDelaySec >= remSec) {
                    awayElapsedSec = offDelaySec - remSec;
                }
            }

            http.end();
            return true;
        }

        if (!isCommand) {
            online = false;
            _cachedResolvedBase = "";
        }
        http.end();
        return false;
    }
};

extern LampClient lampClient;

#endif // LAMP_CLIENT_H