#pragma once

#include <init_core.h>

namespace asm8086 {

enum struct LogLevel : u8 {
    L_TRACE = 0,
    L_DEBUG,
    L_INFO,
    L_WARNING,
    L_ERROR,
    L_FATAL,

    SENTINEL
};

enum struct LogSpecialMode : u8 {
    NONE = 0,

    SECTION_TITLE,

    SENTINEL
};

bool initLoggingSystem(LogLevel minLogLevel);
void shutdownLoggingSystem();

void __log(LogLevel level, LogSpecialMode mode, const char* funcName, const char* format, ...);
void muteLogger(bool mute);

#define logTrace(format, ...) __log(asm8086::LogLevel::L_TRACE,   asm8086::LogSpecialMode::NONE, __func__, format, ##__VA_ARGS__)
#define logDebug(format, ...) __log(asm8086::LogLevel::L_DEBUG,   asm8086::LogSpecialMode::NONE, __func__, format, ##__VA_ARGS__)
#define logInfo(format, ...)  __log(asm8086::LogLevel::L_INFO,    asm8086::LogSpecialMode::NONE, __func__, format, ##__VA_ARGS__)
#define logWarn(format, ...)  __log(asm8086::LogLevel::L_WARNING, asm8086::LogSpecialMode::NONE, __func__, format, ##__VA_ARGS__)
#define logErr(format, ...)   __log(asm8086::LogLevel::L_ERROR,   asm8086::LogSpecialMode::NONE, __func__, format, ##__VA_ARGS__)
#define logFatal(format, ...) __log(asm8086::LogLevel::L_FATAL,   asm8086::LogSpecialMode::NONE, __func__, format, ##__VA_ARGS__)

void writeLine(const char* format, ...);
void writeDirect(const char* format, ...);
void writeLineBold(const char* format, ...);
void writeDirectBold(const char* format, ...);

} // namespace stlv

