#ifndef SYS_LOGGER_H
#define SYS_LOGGER_H

#include <Arduino.h>

enum LogLevel {
    LOG_INFO = 0, // Grey on display
    LOG_NET  = 1, // Cyan on display
    LOG_ERR  = 2  // Grapefruit on display
};

struct LogEntry {
    String text;
    LogLevel level;
};

class SysLogger {
public:
    static const uint8_t MAX_LOGS = 10;

private:
    LogEntry _buffer[MAX_LOGS];
    uint8_t _count;

    String getTimestamp();

public:
    SysLogger() : _count(0) {}

    void log(const String &msg, LogLevel level = LOG_INFO);
    uint8_t getCount() const { return _count; }
    LogEntry getEntry(uint8_t index) const;
};

extern SysLogger sys_log;

#endif // SYS_LOGGER_H