#include "sys_logger.h"
#include <sys/time.h>
#include <time.h>

SysLogger sys_log;

static String pad2(uint32_t val) {
    return (val < 10 ? "0" : "") + String(val);
}

String SysLogger::getTimestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *t = localtime(&tv.tv_sec);

    if (t->tm_year > (2020 - 1900)) {
        return "[" + pad2(t->tm_hour) + ":" + pad2(t->tm_min) + ":" + pad2(t->tm_sec) + "]";
    } else {
        uint32_t sec = millis() / 1000;
        return "[" + pad2((sec / 60) % 100) + ":" + pad2(sec % 60) + "]";
    }
}

void SysLogger::log(const String &msg, LogLevel level) {
    String full_line = getTimestamp() + " " + msg;
    Serial.println(full_line);

    if (_count < MAX_LOGS) {
        _buffer[_count].text = full_line;
        _buffer[_count].level = level;
        _count++;
    } else {
        for (uint8_t i = 0; i < MAX_LOGS - 1; i++) {
            _buffer[i] = _buffer[i + 1];
        }
        _buffer[MAX_LOGS - 1].text = full_line;
        _buffer[MAX_LOGS - 1].level = level;
    }
}

LogEntry SysLogger::getEntry(uint8_t index) const {
    if (index < _count) {
        return _buffer[index];
    }
    return {"", LOG_INFO};
}