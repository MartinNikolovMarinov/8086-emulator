#include <init_core.h>
#include <logger.h>

#include <stdio.h>

void debugPrintCmdArguments(asm8086::CommandLineArguments& args) {
    asm8086::logClean("Command line arguments:\n");
    asm8086::logClean("\tFile name: {}\n", args.fileName);
    asm8086::logClean("\tExec flag: {}\n", args.execFlag);
    asm8086::logClean("\tVerbose flag: {}\n", args.verboseFlag);
    asm8086::logClean("\tDump memory: {}\n", args.dumpMemory);
    asm8086::logClean("\tDump start: {}\n", args.dumpStart);
    asm8086::logClean("\tDump end: {}\n", args.dumpEnd);
    asm8086::logClean("\tImmediate values format: {}\n", args.immValuesFmt);
}

i32 main(i32 argc, char const** argv) {
    if (!asm8086::initLoggingSystem(asm8086::LogLevel::L_TRACE)) {
        fprintf(stderr, "Failed to initialize the logging system.\n");
        return -1;
    }

    if (!initCore(argc, argv)) {
        logErr("Failed to initialize.");
        return -1;
    }

    debugPrintCmdArguments(asm8086::cmdArgs);

    return 0;
}
