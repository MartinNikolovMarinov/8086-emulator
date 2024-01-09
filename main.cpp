#include <init_core.h>
#include <logger.h>
#include <decoder.h>
#include <emulator.h>

#include <stdio.h>

struct CommandLineArguments {
    core::StrBuilder<> fileName;
    i32 fileNameLen = 0;
    bool execFlag = false;
    bool verboseFlag = false;
    bool dumpMemory = false;
    u32 dumpStart = 0;
    u32 dumpEnd = u32(core::MEGABYTE);
    i32 immValuesFmt = 0;

    bool isVerbose() const { return verboseFlag && !dumpMemory; }
};

CommandLineArguments cmdArgs;

void printUsage() {
    using namespace asm8086;

    logCleanBold("Usage:");
    logClean("  -f (required)       the binary file to use.");
    logClean("  --exec              emulate the execution.");
    logClean("  --verbose           print verbose information.");
    logClean("  --dump-memory       dumps the memory to standard out. When this option is on, all other std output is off.");
    logClean("  -dump-start         the start address of the memory dump. Requires dump-memory to be set to true.");
    logClean("                      If not specified, the default is 0.");
    logClean("                      Must be less than dump-end.");
    logClean("  -dump-end           the end address of the memory dump. Requires dump-memory to be set to true.");
    logClean("                      If not specified, the default is 1024*1024.");
    logClean("                      Must be greater than dump-start.");
    logClean("  -imm-values-fmt     the format to use when printing immediate values.");
    logClean("                      0 - is the default.");
    logClean("                      1 - use hex format.");
    logClean("                      2 - use signed format.");
    logClean("                      3 - use unsigned format.");
}

bool parseCmdArguments(i32 argc, char const** argv) {
    bool argsAreOk = false;
    if (argc > 1) {
        core::CmdFlagParser parser;
        parser.allowUnknownFlags(true);

        parser.setFlagString(&cmdArgs.fileName, core::sv("f"), true);
        parser.setFlagBool(&cmdArgs.dumpMemory, core::sv("dump-memory"), false);
        parser.setFlagUint32(&cmdArgs.dumpStart, core::sv("dump-start"), false, [](void* a) -> bool {
            u32 v = *reinterpret_cast<u32*>(a);
            return (v < cmdArgs.dumpEnd);
        });
        parser.setFlagUint32(&cmdArgs.dumpEnd, core::sv("dump-end"), false, [](void* a) -> bool {
            u32 v = *reinterpret_cast<u32*>(a);
            return (v > cmdArgs.dumpStart);
        });
        parser.setFlagInt32(&cmdArgs.immValuesFmt, core::sv("imm-values-fmt"), false, [](void* a) -> bool {
            i32 v = *reinterpret_cast<i32*>(a);
            return (v >= 0 && v <= 3);
        });

        {
            auto res = parser.parse(addr_size(argc), argv);
            argsAreOk = !res.hasErr();
        }

        {
            auto res = parser.matchFlags();
            argsAreOk = !res.hasErr();
        }

        // Check set options
        {
            parser.options([](const core::StrView arg) {
                if (arg.eq(core::sv("exec"))) {
                    cmdArgs.execFlag = true;
                }
                else if (arg.eq(core::sv("verbose"))) {
                    cmdArgs.verboseFlag = true;
                }

                return true;
            });
        }
    }

    if (!argsAreOk) {
        printUsage();
        return false;
    }

    return true;
}

void debugPrintCmdArguments(CommandLineArguments& args) {
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

void dumpMemory(u8* memory, u32 start, u32 end) {
    i64 bytesWritten;
    core::FileDesc standardOut;
    standardOut.handle = reinterpret_cast<void*>(core::STDOUT);
    core::fileWrite(standardOut, memory + start, addr_size(end - start));
    Assert(bytesWritten == end);
}

void printRegisterState(asm8086::EmulationContext& ctx) {
    using RegisterType = asm8086::RegisterType;

    asm8086::logClean("Final Registers:");

    auto reg = ctx.registers[i32(RegisterType::AX)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::BX)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::CX)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DX)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SP)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::BP)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SI)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DI)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);

    asm8086::logClean("");

    reg = ctx.registers[i32(RegisterType::ES)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SS)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DS)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::CS)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);

    asm8086::logClean("");

    reg = ctx.registers[i32(RegisterType::IP)];
    asm8086::logClean("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);

    reg = ctx.registers[i32(RegisterType::FLAGS)];
    {
        char flagsBuf[asm8086::BUFFER_SIZE_FLAGS] = {};
        flagsToCptr(asm8086::Flags(reg.value), flagsBuf);
        asm8086::logClean("\t%s: %s (%u)", regTypeToCptr(reg.type), flagsBuf, reg.value);
    }
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

    if (!parseCmdArguments(argc, argv)) {
        logErr("Failed to parse command line arguments.");
        return -1;
    }
    debugPrintCmdArguments(cmdArgs);

    core::Arr<u8> binaryData;
    Expect(core::fileReadEntire(cmdArgs.fileName.view().data(), binaryData));

    asm8086::DecodingContext ctx = {};
    switch (cmdArgs.immValuesFmt) {
        case 1: ctx.options = asm8086::DecodingOpts(ctx.options | asm8086::DEC_OP_IMMEDIATE_AS_HEX); break;
        case 2: ctx.options = asm8086::DecodingOpts(ctx.options | asm8086::DEC_OP_IMMEDIATE_AS_SIGNED); break;
        case 3: ctx.options = asm8086::DecodingOpts(ctx.options | asm8086::DEC_OP_IMMEDIATE_AS_UNSIGNED); break;
    }

    asm8086::decodeAsm8086(binaryData, ctx);

    core::StrBuilder sb;
    if (!cmdArgs.execFlag || cmdArgs.isVerbose()) {
        asm8086::encodeAsm8086(sb, ctx);
        asm8086::logClean(sb.view().data());
    }

    if (cmdArgs.execFlag) {
        sb.clear();

        asm8086::EmulationContext emuCtx = asm8086::createEmulationCtx(core::move(ctx.instructions));
        emuCtx.__verbosecity_buff = core::move(sb);
        if (cmdArgs.isVerbose()) {
            emuCtx.emuOpts = asm8086::EmulationOpts(emuCtx.emuOpts | asm8086::EmulationOpts::EMU_OPT_VERBOSE);
        }

        if (cmdArgs.isVerbose()) asm8086::logClean("");
        asm8086::emulate(emuCtx);
        if (cmdArgs.isVerbose()) asm8086::logClean("");

        if (cmdArgs.dumpMemory) {
            dumpMemory(emuCtx.memory, cmdArgs.dumpStart, cmdArgs.dumpEnd);
        }
        else {
            printRegisterState(emuCtx);
        }
    }

    return 0;
}
