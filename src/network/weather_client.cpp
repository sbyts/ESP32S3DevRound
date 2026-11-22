#include "weather_client.h"
#include "app_config.h"
#include "../display/ui_manager.h"
#include "../system/sys_logger.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>

WeatherClient weatherClient;

void WeatherClient::fetchNow()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        _status = "OFFLINE";
        return;
    }

    HTTPClient http;
    http.setTimeout(3000);
    http.setReuse(false);
    http.setUserAgent("curl/7.68.0");

    String clean_city = appConfig.city;
    clean_city.replace(" ", "+");

    // Force metric units (?m) so wttr.in always returns Celsius and km/h
    String uri = "/" + clean_city + "?m&format=%t|%h|%w|%C";

    if (http.begin("wttr.in", 80, uri))
    {
        int httpCode = http.GET();
        if (httpCode == 200)
        {
            String payload = http.getString();
            payload.trim();
            http.end();

            int sep1 = payload.indexOf('|');
            int sep2 = payload.indexOf('|', sep1 + 1);
            int sep3 = payload.indexOf('|', sep2 + 1);

            if (sep1 > 0 && sep2 > sep1 && sep3 > sep2)
            {
                String temp_str = payload.substring(0, sep1);
                String hum_str  = payload.substring(sep1 + 1, sep2);
                String wind_str = payload.substring(sep2 + 1, sep3);
                String cond_str = payload.substring(sep3 + 1);
                cond_str.trim();

                bool is_fahrenheit = (temp_str.indexOf('F') >= 0);
                temp_str.replace("+", "");
                temp_str.replace("°C", "");
                temp_str.replace("°F", "");
                temp_str.replace("°", "");
                hum_str.replace("%", "");

                String clean_wind = "";
                for (size_t i = 0; i < wind_str.length(); i++)
                {
                    char c = wind_str.charAt(i);
                    if (isDigit(c) || c == '.')
                    {
                        clean_wind += c;
                    }
                }

                float out_temp = temp_str.toFloat();
                if (is_fahrenheit)
                {
                    out_temp = (out_temp - 32.0f) * 5.0f / 9.0f;
                }

                float out_hum = hum_str.toFloat();
                float wind_ms = clean_wind.toFloat() / 3.6f;

                String upper_city = appConfig.city;
                upper_city.toUpperCase();

                ui.setOutdoorData(out_temp, out_hum, wind_ms, cond_str.c_str(), upper_city.c_str());
                _status = "OK: " + cond_str + " (" + String((int)round(out_temp)) + "C)";
                sys_log.log("Weather: " + upper_city + " " + String((int)round(out_temp)) + "C", LOG_NET);
            }
            else
            {
                _status = "BAD FORMAT";
                sys_log.log("Weather parse error", LOG_ERR);
            }
        }
        else
        {
            _status = "HTTP ERR: " + String(httpCode);
            sys_log.log("Weather HTTP " + String(httpCode), LOG_ERR);
            http.end();
        }
    }
    else
    {
        _status = "CONN FAILED";
        sys_log.log("Weather conn failed", LOG_ERR);
    }

    _lastUpdateMs = millis();
}

void WeatherClient::process(bool wifiSuspended)
{
    if (WiFi.status() == WL_CONNECTED && !wifiSuspended)
    {
        if (millis() - _lastUpdateMs > WEATHER_INTERVAL)
        {
            fetchNow();
        }
    }
}