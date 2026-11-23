#include "sys_logger.h"
#include <sys/time.h>
#include <time.h>
#include <utility>

SysLogger sys_log;

String SysLogger::getTimestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *t = localtime(&tv.tv_sec);

    char buf[16];
    if (t && t->tm_year > (2020 - 1900)) {
        snprintf(buf, sizeof(buf), "[%02d:%02d:%02d]", t->tm_hour, t->tm_min, t->tm_sec);
    } else {
        uint32_t sec = millis() / 1000;
        snprintf(buf, sizeof(buf), "[%02lu:%02lu]", (unsigned long)((sec / 60) % 100), (unsigned long)(sec % 60));
    }
    return String(buf);
}

void SysLogger::log(const String &msg, LogLevel level) {
    // Filter out noisy screen wake-up messages
    if (msg == "Button press -> Screen ON") {
        return;
    }

    // Prepare string outside the critical section to avoid heap operations while interrupts are disabled
    String full_line = getTimestamp();
    full_line += " ";
    full_line += msg;

    Serial.println(full_line);

    portENTER_CRITICAL(&_mux);
    if (_count < MAX_LOGS) {
        _buffer[_count].text = std::move(full_line);
        _buffer[_count].level = level;
        _count++;
    } else {
        for (uint8_t i = 0; i < MAX_LOGS - 1; i++) {
            _buffer[i].text = std::move(_buffer[i + 1].text);
            _buffer[i].level = _buffer[i + 1].level;
        }
        _buffer[MAX_LOGS - 1].text = std::move(full_line);
        _buffer[MAX_LOGS - 1].level = level;
    }
    portEXIT_CRITICAL(&_mux);
}

uint8_t SysLogger::getCount() const {
    portENTER_CRITICAL(&_mux);
    uint8_t c = _count;
    portEXIT_CRITICAL(&_mux);
    return c;
}

LogEntry SysLogger::getEntry(uint8_t index) const {
    LogEntry entry = {"", LOG_INFO};
    portENTER_CRITICAL(&_mux);
    if (index < _count) {
        entry = _buffer[index];
    }
    portEXIT_CRITICAL(&_mux);
    return entry;
}