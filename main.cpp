#include <init_core.h>
#include <logger.h>
#include <decoder.h>
#include <emulator.h>

#include <stdio.h>
#include <iostream>


// TODO:
// General list of unfinished things, that would be easy to do:
//
// * I should load the instruction bytes into memory as well. Then fix the terrible linear search for jump instructions.
// * The encoder has some bugs, where it does not encode instruction sizes (word/byte keywords) correctly.
// * Better error handling should not allow any crashes, at least in the decoder/encoder logic.
// * Support encoding and decoding for the entire 8086 instruction set. This is a bit tedious, but shouldn't be hard at
//   this point. All the facilities are there.
// * Segment register to memory and memory to segment register instructions should be implemented.
//   This should be very easy.
// * Trap, interrupt and direction FLAGS are missing.
// * The emulator should be able to handle processor control instructions. Most of them are quite simple:
//    * clc - just clear the carry flag
//    * cmc - just toggle the carry flag
//    * stc - just set the carry flag
//    * cld - set the direction flag to 0, the direction flag is not used yet
//    * std - set the direction flag to 1, the direction flag is not used yet
//    * cli - clear the interrupt flag, interrupts are not used yet
//    * sti - set the interrupt flag, interrupts are not used yet
//    * hlt - just exit the emulator
//    * esc - nothing to do for esc really. Unless I decide to implement an external FPU unit emulator, or somthing even crazier.
//    * lock - this is used in multi-processor systems, so not really applicable for an emulator.
//    * segment - override prefix. This one is probably the hardest because it requires a full implementation of the
//      8086 memory addressing mechanisms.
//
// TODO2:
// Features missing from the emulator:
//
// * Procedures - would be nice to have enough instruction support to create a procedure calling convention.
// * Addressing of the full Megabyte of memory - this requires a full implementation of 8086 memory addressing. Which is
//   significant amount of work for little educational benefits, because modern hardware does not use a similar model.
// * String operation commands - I don't think I will be supporting strings for the forseeable future.
// * Interrupts - calling operating system interupts requires an operating system :) This could however be used for
//   printing to std in and out. Maybe implementing just that one interupt would be cool.
// * Other missing instruction that would be necessary for an actually usable emulator:
//    * Data Transfer:
//        * in/out - input/output to/from a port
//        * lea - load effective address
//        * push/pop - push/pop a value from the stack
//        * xchg - exchange the contents of two registers
//    * Arithmetic:
//        * inc/dec - increment/decrement a register
//        * mul/imul - multiply a register by another register or a value
//        * div/idiv - divide a register by another register or a value
//    * Logical:
//        * all logic instructions are kinda necessary for a complete emulator.
//    * String manipulation:
//        * rep - repeat the next instruction
//        * movs - move a string
//        * cmps - compare two strings
//        * scas - scan a string
//        * lods - load a string
//        * stds - store a string
//     * Control Transfer (these are a necessary requiremnt, since they are used for procedure calls):
//        * call - call a procedure
//        * ret - return from a procedure
//        * jmp - long jump to a label

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

    writeLineBold("Usage:");
    writeLine("  -f (required)       the binary file to use.");
    writeLine("  --exec              emulate the execution.");
    writeLine("  --verbose           print verbose information.");
    writeLine("  --dump-memory       dumps the memory to standard out. When this option is on, all other std output is off.");
    writeLine("  -dump-start         the start address of the memory dump. Requires dump-memory to be set to true.");
    writeLine("                      If not specified, the default is 0.");
    writeLine("                      Must be less than dump-end.");
    writeLine("  -dump-end           the end address of the memory dump. Requires dump-memory to be set to true.");
    writeLine("                      If not specified, the default is 1024*1024.");
    writeLine("                      Must be greater than dump-start.");
    writeLine("  -imm-values-fmt     the format to use when printing immediate values.");
    writeLine("                      0 - is the default.");
    writeLine("                      1 - use hex format.");
    writeLine("                      2 - use signed format.");
    writeLine("                      3 - use unsigned format.");
}

bool parseCmdArguments(i32 argc, char const** argv) {
    bool argsAreOk = false;
    if (argc > 1) {
        core::CmdFlagParser parser;
        parser.allowUnknownFlags(true);

        parser.setFlagString(&cmdArgs.fileName, core::sv("f"), true);
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
                else if (arg.eq(core::sv("dump-memory"))) {
                    cmdArgs.dumpMemory = true;
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
    std::cout.write(reinterpret_cast<char*>(memory + start),  end - start);
}

void printRegisterState(asm8086::EmulationContext& ctx) {
    using RegisterType = asm8086::RegisterType;

    asm8086::writeLine("Final Registers:");

    auto reg = ctx.registers[i32(RegisterType::AX)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::BX)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::CX)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DX)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SP)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::BP)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SI)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DI)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);

    asm8086::writeLine("");

    reg = ctx.registers[i32(RegisterType::ES)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SS)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DS)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::CS)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);

    asm8086::writeLine("");

    reg = ctx.registers[i32(RegisterType::IP)];
    asm8086::writeLine("\t%s: 0x%06X (%u)", regTypeToCptr(reg.type), reg.value, reg.value);

    reg = ctx.registers[i32(RegisterType::FLAGS)];
    {
        char flagsBuf[asm8086::BUFFER_SIZE_FLAGS] = {};
        flagsToCptr(asm8086::Flags(reg.value), flagsBuf);
        asm8086::writeLine("\t%s: %s (%u)", regTypeToCptr(reg.type), flagsBuf, reg.value);
    }
}

i32 main(i32 argc, char const** argv) {
    if (!asm8086::initLoggingSystem(asm8086::LogLevel::L_INFO)) {
        fprintf(stderr, "Failed to initialize the logging system.\n");
        return -1;
    }

    if (!initCore()) {
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
        asm8086::writeLine(sb.view().data());
    }

    if (cmdArgs.execFlag) {
        sb.clear();

        asm8086::EmulationContext emuCtx = asm8086::createEmulationCtx(core::move(ctx.instructions));
        emuCtx.__verbosecity_buff = core::move(sb);
        if (cmdArgs.isVerbose()) {
            emuCtx.emuOpts = asm8086::EmulationOpts(emuCtx.emuOpts | asm8086::EmulationOpts::EMU_OPT_VERBOSE);
        }

        asm8086::emulate(emuCtx);
        if (cmdArgs.isVerbose()) asm8086::writeLine("");

        if (cmdArgs.dumpMemory) {
            dumpMemory(emuCtx.memory, cmdArgs.dumpStart, cmdArgs.dumpEnd);
        }
        else {
            printRegisterState(emuCtx);
        }
    }

    return 0;
}
