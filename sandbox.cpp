#include <init_core.h>
#include <logger.h>

#include <stdio.h>

void debugPrintCmdArguments(asm8086::CommandLineArguments& args) {
    logDebug(
            "Command line arguments\n"
            "\tFile name: %s\n"
            "\tExec flag: %s\n"
            "\tVerbose flag: %s\n"
            "\tDump memory: %s\n"
            "\tDump start: %u\n"
            "\tDump end: %u\n"
            "\tImmediate values format: %d",

            args.fileName.view().data(),
            args.execFlag ? "true" : "false",
            args.verboseFlag ? "true" : "false",
            args.dumpMemory ? "true" : "false",
            args.dumpStart,
            args.dumpEnd,
            args.immValuesFmt
    );
}

i32 main(i32 argc, char const** argv) {
    if (!asm8086::initLoggingSystem(asm8086::LogLevel::L_INFO)) {
        fprintf(stderr, "Failed to initialize the logging system.\n");
        return -1;
    }

    if (!initCore(argc, argv)) {
        logErr("Failed to initialize.");
        return -1;
    }
    debugPrintCmdArguments(asm8086::cmdArgs);

    core::Arr<u8> binaryData;
    if (auto res = core::fileReadEntire(asm8086::cmdArgs.fileName.view().data(), binaryData); res.hasErr()) {
        logErr("Failed to read file: %d", res.err());
        return -1;
    }

    return 0;
}
