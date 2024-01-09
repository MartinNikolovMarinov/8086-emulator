#include <init_core.h>
#include <logger.h>

#include <stdexcept>
#include <stdio.h>

// Hashing functions for some core types:

template<>
addr_size core::hash(const core::StrView& key) {
    addr_size h = addr_size(core::simpleHash_32(key.data(), key.len()));
    return h;
}

template<>
addr_size core::hash(const i32& key) {
    addr_size h = addr_size(core::simpleHash_32(reinterpret_cast<const void*>(&key), sizeof(key)));
    return h;
}

template<>
addr_size core::hash(const u32& key) {
    addr_size h = addr_size(core::simpleHash_32(reinterpret_cast<const void*>(&key), sizeof(key)));
    return h;
}

template<>
bool core::eq(const core::StrView& a, const core::StrView& b) {
    return a.eq(b);
}

template<>
bool core::eq(const i32& a, const i32& b) {
    return a == b;
}

template<>
bool core::eq(const u32& a, const u32& b) {
    return a == b;
}

void printUsage() {
    using namespace asm8086;

    logCleanBold("Usage:");
    logClean("  -f (required)       the binary file to use.");
    logClean("  --exec              emulate the execution.");
    logClean("  --verbose           print verbose information.");
    logClean("  --dump-memory       dumps the memory to standard out. When this option is on, all other std output is off.");
    logClean("  --dump-start        the start address of the memory dump. Requires dump-memory to be set to true.");
    logClean("                      If not specified, the default is 0.");
    logClean("                      Must be less than dump-end.");
    logClean("  --dump-end          the end address of the memory dump. Requires dump-memory to be set to true.");
    logClean("                      If not specified, the default is 1024*1024.");
    logClean("                      Must be greater than dump-start.");
    logClean("  --imm-values-fmt    the format to use when printing immediate values.");
    logClean("                      0 - is the default.");
    logClean("                      1 - use hex format.");
    logClean("                      2 - use signed format.");
    logClean("                      3 - use unsigned format.");
}

namespace asm8086 { CommandLineArguments cmdArgs; }


bool initCore(i32 argc, const char** argv) {
    using namespace asm8086;

    core::setGlobalAssertHandler([](const char* failedExpr, const char* file, i32 line, const char* funcName, const char* errMsg) {
        constexpr u32 stackFramesToSkip = 3;
        constexpr addr_size stackTraceBufferSize = 4096;
        char trace[stackTraceBufferSize] = {};
        addr_size traceLen = 0;
        core::stacktrace(trace, stackTraceBufferSize, traceLen, 200, stackFramesToSkip);

        fprintf(stderr,
                ANSI_BOLD(ANSI_RED("[ASSERTION] [EXPR]:")) ANSI_BOLD(" %s\n")
                ANSI_BOLD(ANSI_RED("[FUNC]:"))             ANSI_BOLD(" %s\n")
                ANSI_BOLD(ANSI_RED("[FILE]:"))             ANSI_BOLD(" %s:%d\n")
                ANSI_BOLD(ANSI_RED("[MSG]:"))              ANSI_BOLD(" %s\n"),

                failedExpr, funcName, file, line, errMsg
        );

        fprintf(stderr, ANSI_BOLD("[TRACE]:\n%s\n"), trace);

        throw std::runtime_error("Assertion failed!");
    });

    bool argsAreOk = false;
    if (argc > 1) {
        core::CmdFlagParser parser;
        parser.allowUnknownFlags(true);

        parser.setFlagString(&cmdArgs.fileName, core::sv("f"), true);
        parser.setFlagBool(&cmdArgs.execFlag, core::sv("exec"), false);
        parser.setFlagBool(&cmdArgs.verboseFlag, core::sv("verbose"), false);
        parser.setFlagBool(&cmdArgs.dumpMemory, core::sv("dump-memory"), false);
        parser.setFlagUint32(&cmdArgs.dumpStart, core::sv("dump-start"), false, [](void* a) -> bool {
            u32 v = *(u32*)a;
            return (v < cmdArgs.dumpEnd);
        });
        parser.setFlagUint32(&cmdArgs.dumpEnd, core::sv("dump-end"), false, [](void* a) -> bool {
            u32 v = *(u32*)a;
            return (v > cmdArgs.dumpStart);
        });

        {
            auto res = parser.parse(argc, argv);
            argsAreOk = !res.hasErr();
        }
        {
            auto res = parser.matchFlags();
            argsAreOk = !res.hasErr();
        }
    }

    if (!argsAreOk) {
        printUsage();
        return false;
    }

    return true;
}
