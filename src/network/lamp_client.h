#ifndef LAMP_CLIENT_H
#define LAMP_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "../config/config.h"
#include "../peripherals/radar_ld2410.h"
#include "../system/sys_logger.h"

class LampClient {
public:
    // Live lamp status
    bool online = false;
    bool isOn = false;
    uint8_t mode = 1;           // 1: 3000K, 2: 6000K, 3: 4500K
    int brightness = 80;        // 10..100%

    // Presence automation settings
    bool autoPresence = true;
    uint32_t offDelaySec = DEFAULT_LAMP_DELAY;  // Configurable delay before turning lamp off (seconds)
    bool userPresent = false;
    uint32_t awayElapsedSec = 0;

    void begin(const String& baseUrl = DEFAULT_LAMP_URL,
               const String& linkKey = DEFAULT_LAMP_KEY,
               uint32_t delaySec = DEFAULT_LAMP_DELAY) {
        _baseUrl = baseUrl;
        _baseUrl.trim();
        if (_baseUrl.endsWith("/")) {
            _baseUrl.remove(_baseUrl.length() - 1);
        }
        // Strip /api/link if user entered full endpoint path in Web UI
        if (_baseUrl.endsWith("/api/link")) {
            _baseUrl.remove(_baseUrl.length() - 9);
        }

        _key = linkKey;
        offDelaySec = delaySec;
    }

    // Call in loop() with raw radar pin state (PIN_RADAR_OUT)
    void updatePresence(bool rawPinHigh) {
        if (!autoPresence || !radar.isEnabled()) {
            userPresent = false;
            awayElapsedSec = 0;
            return;
        }

        unsigned long now = millis();

        if (rawPinHigh) {
            userPresent = true;
            _awayStartMs = 0;
            awayElapsedSec = 0;

            if (!_sentPresentState) {
                if (sendCommand(true)) {
                    _sentPresentState = true;
                    isOn = true;
                    sys_log.log("Lamp Link -> Presence ON", LOG_INFO);
                }
            }
        } else {
            userPresent = false;
            if (_sentPresentState) {
                if (_awayStartMs == 0) {
                    _awayStartMs = now;
                }
                awayElapsedSec = (now - _awayStartMs) / 1000;

                if (awayElapsedSec >= offDelaySec) {
                    if (sendCommand(false)) {
                        _sentPresentState = false;
                        _awayStartMs = 0;
                        awayElapsedSec = 0;
                        isOn = false;
                        sys_log.log("Lamp Link -> Away timeout OFF", LOG_INFO);
                    }
                }
            }
        }
    }

    // Periodic background sync (every 3s when Wi-Fi is active)
    void pollStatus() {
        if (millis() - _lastPollMs < 3000) return;
        _lastPollMs = millis();

        if (WiFi.status() != WL_CONNECTED) {
            online = false;
            return;
        }
        executeHttp("GET", resolveEndpointUrl());
    }

private:
    String _baseUrl = DEFAULT_LAMP_URL;
    String _key = DEFAULT_LAMP_KEY;
    bool _sentPresentState = false;
    unsigned long _awayStartMs = 0;
    unsigned long _lastPollMs = 0;

    // Resolve .local mDNS hostname to IP if needed and append /api/link
    String resolveEndpointUrl() {
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
                String mdnsName = host.substring(0, host.length() - 6);
                IPAddress ip = MDNS.queryHost(mdnsName.c_str(), 250);
                if (ip != IPAddress(0, 0, 0, 0)) {
                    resolvedBase = "http://" + ip.toString() + portPart;
                }
            }
        }
        return resolvedBase + "/api/link";
    }

    bool sendCommand(bool turnOn) {
        if (WiFi.status() != WL_CONNECTED) return false;
        String cmdUrl = resolveEndpointUrl() + "?presence=" + (turnOn ? "1" : "0");
        return executeHttp("POST", cmdUrl);
    }

    bool executeHttp(const char* method, const String& targetUrl) {
        WiFiClient client;
        HTTPClient http;
        http.setConnectTimeout(300);
        http.setTimeout(300);

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

            http.end();
            return true;
        }

        online = false;
        http.end();
        return false;
    }
};

extern LampClient lampClient;

#endif // LAMP_CLIENT_H