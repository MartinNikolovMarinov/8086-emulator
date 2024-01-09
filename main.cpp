// #include "src/init_core.h"
// #include "src/emulator.h"

// #include <fcntl.h>

// using namespace asm8086;

// // TODO:
// // General list of unfinished things, that would be easy to do:
// //
// // * I should load the instruction bytes into memory as well. Then fix the terrible linear search for jump instructions.
// // * The encoder has some bugs, where it does not encode instruction sizes (word/byte keywords) correctly.
// // * Better error handling should not allow any crashes, at least in the decoder/encoder logic.
// // * Support encoding and decoding for the entire 8086 instruction set. This is a bit tedious, but shouldn't be hard at
// //   this point. All the facilities are there.
// // * Segment register to memory and memory to segment register instructions should be implemented.
// //   This should be very easy.
// // * Trap, interrupt and direction FLAGS are missing.
// // * The emulator should be able to handle processor control instructions. Most of them are quite simple:
// //    * clc - just clear the carry flag
// //    * cmc - just toggle the carry flag
// //    * stc - just set the carry flag
// //    * cld - set the direction flag to 0, the direction flag is not used yet
// //    * std - set the direction flag to 1, the direction flag is not used yet
// //    * cli - clear the interrupt flag, interrupts are not used yet
// //    * sti - set the interrupt flag, interrupts are not used yet
// //    * hlt - just exit the emulator
// //    * esc - nothing to do for esc really. Unless I decide to implement an external FPU unit emulator, or somthing even crazier.
// //    * lock - this is used in multi-processor systems, so not really applicable for an emulator.
// //    * segment - override prefix. This one is probably the hardest because it requires a full implementation of the
// //      8086 memory addressing mechanisms.
// //
// // TODO2:
// // Features missing from the emulator:
// //
// // * Procedures - would be nice to have enough instruction support to create a procedure calling convention.
// // * Addressing of the full Megabyte of memory - this requires a full implementation of 8086 memory addressing. Which is
// //   significant amount of work for little educational benefits, because modern hardware does not use a similar model.
// // * String operation commands - I don't think I will be supporting strings for the forseeable future.
// // * Interrupts - calling operating system interupts requires an operating system :) This could however be used for
// //   printing to std in and out. Maybe implementing just that one interupt would be cool.
// // * Other missing instruction that would be necessary for an actually usable emulator:
// //    * Data Transfer:
// //        * in/out - input/output to/from a port
// //        * lea - load effective address
// //        * push/pop - push/pop a value from the stack
// //        * xchg - exchange the contents of two registers
// //    * Arithmetic:
// //        * inc/dec - increment/decrement a register
// //        * mul/imul - multiply a register by another register or a value
// //        * div/idiv - divide a register by another register or a value
// //    * Logical:
// //        * all logic instructions are kinda necessary for a complete emulator.
// //    * String manipulation:
// //        * rep - repeat the next instruction
// //        * movs - move a string
// //        * cmps - compare two strings
// //        * scas - scan a string
// //        * lods - load a string
// //        * stds - store a string
// //     * Control Transfer (these are a necessary requiremnt, since they are used for procedure calls):
// //        * call - call a procedure
// //        * ret - return from a procedure
// //        * jmp - long jump to a label

// void printRegisterState(EmulationContext& ctx) {
//     fmt::print("Final Registers:\n");

//     auto reg = ctx.registers[i32(RegisterType::AX)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::BX)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::CX)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::DX)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::SP)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::BP)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::SI)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::DI)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);

//     fmt::print("\n");

//     reg = ctx.registers[i32(RegisterType::ES)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::SS)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::DS)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
//     reg = ctx.registers[i32(RegisterType::CS)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);

//     fmt::print("\n");

//     reg = ctx.registers[i32(RegisterType::IP)];
//     fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);

//     reg = ctx.registers[i32(RegisterType::FLAGS)];
//     {
//         char flagsBuf[BUFFER_SIZE_FLAGS] = {};
//         flagsToCptr(Flags(reg.value), flagsBuf);
//         fmt::print("\t{}: {} ({})\n", regTypeToCptr(reg.type), flagsBuf, reg.value);
//     }
// }

// void dumpMemory(u8* memory, u32 start, u32 end) {
//     i64 bytesWritten;
//     core::os_write(core::file_desc{(void*)core::STDOUT}, memory + start, end, bytesWritten);
//     Assert(bytesWritten == end);
// }

// void debugPrintCmdArguments(command_line_args& args) {
//     fmt::print("Command line arguments:\n");
//     fmt::print("\tFile name: {}\n", args.fileName);
//     fmt::print("\tExec flag: {}\n", args.execFlag);
//     fmt::print("\tVerbose flag: {}\n", args.verboseFlag);
//     fmt::print("\tDump memory: {}\n", args.dumpMemory);
//     fmt::print("\tDump start: {}\n", args.dumpStart);
//     fmt::print("\tDump end: {}\n", args.dumpEnd);
//     fmt::print("\tImmediate values format: {}\n", args.immValuesFmt);
// }

// i32 main(i32 argc, char const** argv) {
//     command_line_args cmdLineArgs = initCore(argc, argv);

//     core::arr<u8> binaryData;
//     Expect(core::os_read_entire_file(cmdLineArgs.fileName, binaryData));

//     core::str_builder<> sb;
//     DecodingContext ctx = {};
//     switch (cmdLineArgs.immValuesFmt) {
//         case 1: ctx.options = DecodingOpts(ctx.options | DEC_OP_IMMEDIATE_AS_HEX); break;
//         case 2: ctx.options = DecodingOpts(ctx.options | DEC_OP_IMMEDIATE_AS_SIGNED); break;
//         case 3: ctx.options = DecodingOpts(ctx.options | DEC_OP_IMMEDIATE_AS_UNSIGNED); break;
//     }

//     decodeAsm8086(binaryData, ctx);

//     if (!cmdLineArgs.execFlag || cmdLineArgs.isVerbose()) {
//         encodeAsm8086(sb, ctx);
//         fmt::print("{}", sb.view().buff);
//     }

//     sb.clear();

//     if (cmdLineArgs.execFlag) {
//         EmulationContext emuCtx = createEmulationCtx(core::move(ctx.instructions));
//         emuCtx.__verbosecity_buff = core::move(sb);
//         if (cmdLineArgs.isVerbose()) {
//             emuCtx.emuOpts = EmulationOpts(emuCtx.emuOpts | EmulationOpts::EMU_OPT_VERBOSE);
//         }

//         if (cmdLineArgs.isVerbose()) fmt::print("\n");
//         emulate(emuCtx);
//         if (cmdLineArgs.isVerbose()) fmt::print("\n");

//         if (cmdLineArgs.dumpMemory) {
//             dumpMemory(emuCtx.memory, cmdLineArgs.dumpStart, cmdLineArgs.dumpEnd);
//         }
//         else {
//             printRegisterState(emuCtx);
//         }
//     }

//     return 0;
// }

#include <init_core.h>
#include <logger.h>
#include <decoder.h>

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

bool parseCmdArguments(i32 argc, char const** argv) {
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

i32 main(i32 argc, char const** argv) {
    if (!asm8086::initLoggingSystem(asm8086::LogLevel::L_TRACE)) {
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

    Assert(false, "Not implemented yet.");
    return 0;
}
