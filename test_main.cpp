#include "tests/t-index.h"

#include <stdio.h>

i32 main(i32 argc, char const** argv) {
    if (!asm8086::initLoggingSystem(asm8086::LogLevel::L_INFO)) {
        fprintf(stderr, "Failed to initialize the logging system.\n");
        return -1;
    }

    if (!initCore(argc, argv)) {
        logErr("Failed to initialize.");
        return -1;
    }

    // Print compiler
    if constexpr (COMPILER_CLANG == 1)   { printf("%s\n", "[COMPILER] COMPILER_CLANG");   }
    if constexpr (COMPILER_GCC == 1)     { printf("%s\n", "[COMPILER] COMPILER_GCC");     }
    if constexpr (COMPILER_MSVC == 1)    { printf("%s\n", "[COMPILER] COMPILER_MSVC");    }
    if constexpr (COMPILER_UNKNOWN == 1) { printf("%s\n", "[COMPILER] COMPILER_UNKNOWN"); }

    // Print OS
    if constexpr (OS_WIN == 1)     { printf("%s\n", "[OS] OS_WIN");     }
    if constexpr (OS_LINUX == 1)   { printf("%s\n", "[OS] OS_LINUX");   }
    if constexpr (OS_MAC == 1)     { printf("%s\n", "[OS] OS_MAC");     }
    if constexpr (OS_UNKNOWN == 1) { printf("%s\n", "[OS] OS_UNKNOWN"); }

    // Print CPU architecture
    printf("[CPU ARCH] %s\n", CPU_ARCH);

    i32 exitCode = runAllTests();

    return exitCode;
}
