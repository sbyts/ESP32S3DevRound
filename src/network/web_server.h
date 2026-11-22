#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class WebAdminServer {
public:
    WebAdminServer() : _server(80) {}
    void begin();

private:
    AsyncWebServer _server;
    bool checkAuth(AsyncWebServerRequest *request);
    static String formatUptime(uint32_t sec);
};

extern WebAdminServer webAdmin;

#endif // WEB_SERVER_H