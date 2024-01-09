#include <logger.h>

#include <stdio.h>
#include <stdarg.h>

namespace asm8086 {

namespace {

constexpr addr_size BUFFER_SIZE = core::KILOBYTE * 32;
thread_local static char loggingBuffer[BUFFER_SIZE];

LogLevel minimumLogLevel = LogLevel::L_INFO;
bool muted = false;

} // namespace

bool initLoggingSystem(LogLevel minLogLevel) {
    minimumLogLevel = minLogLevel;
    return true;
}

void shutdownLoggingSystem() {
    minimumLogLevel = LogLevel::L_INFO;
}

void muteLogger(bool mute) {
    muted = mute;
}

void __log(LogLevel level, LogSpecialMode mode, const char* funcName, const char* format, ...) {
    if (level < minimumLogLevel)          return;
    if (muted)                            return;

    loggingBuffer[0] = '\0';

    va_list args;
    va_start(args, format);
    vsnprintf(loggingBuffer, BUFFER_SIZE, format, args);
    va_end(args);

    // Print Level
    switch (level) {
        case LogLevel::L_DEBUG:
            printf(ANSI_BOLD("[DEBUG]"));
            break;
        case LogLevel::L_INFO:
            printf(ANSI_BOLD(ANSI_BRIGHT_BLUE("[INFO]")));
            break;
        case LogLevel::L_WARNING:
            printf(ANSI_BOLD(ANSI_BRIGHT_YELLOW("[WARNING]")));
            break;
        case LogLevel::L_ERROR:
            printf(ANSI_BOLD(ANSI_RED("[ERROR]")));
            break;
        case LogLevel::L_FATAL:
            printf(ANSI_BOLD(ANSI_BACKGROUND_RED(ANSI_BRIGHT_WHITE("[FATAL]"))));
            break;
        case LogLevel::L_TRACE:
            printf(ANSI_BOLD(ANSI_BRIGHT_GREEN("[TRACE]")));
            break;

        case LogLevel::SENTINEL: [[fallthrough]];
        default:
            printf("[UNKNOWN]");
            break;
    }

    // Print Message
    if (mode == LogSpecialMode::SECTION_TITLE) {
        constexpr const char* separator = ANSI_BOLD(ANSI_BRIGHT_WHITE("---------------------------------------------------------------------"));
        printf(" _fn_(%s):\n", funcName);
        printf("%s\n", separator);
        printf("%s\n", loggingBuffer);
        printf("%s\n", separator);
    }
    else {
        printf(" _fn_(%s): %s\n", funcName, loggingBuffer);
    }
}

void logClean(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(loggingBuffer, BUFFER_SIZE, format, args);
    va_end(args);

    printf("%s\n", loggingBuffer);
}

void logCleanNoSpace(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(loggingBuffer, BUFFER_SIZE, format, args);
    va_end(args);

    printf("%s", loggingBuffer);
}

void logCleanBold(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(loggingBuffer, BUFFER_SIZE, format, args);
    va_end(args);

    printf(ANSI_BOLD("%s\n"), loggingBuffer);
}

void logCleanBoldNoSpace(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(loggingBuffer, BUFFER_SIZE, format, args);
    va_end(args);

    printf(ANSI_BOLD("%s"), loggingBuffer);
}

} // namespace stlv
