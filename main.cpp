#include "src/init_core.h"
#include "src/emulator.h"

#include <fcntl.h>

using namespace asm8086;

// TODO:
// General list of unfinished things:
//
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
//
// TODO2:
// Features missing from the emulator:
//
// * Addressing of the full Megabyte of memory. This requires a full implementation of 8086 memory addressing. Which is
//   significant amount of work for little educational benefits, because modern hardware does not use a similar model.
// * String operation commands. I don't think I will be supporting strings for the forseeable future.
// * Interrupts. Calling operating system interupts requires an operating system :) This could however be used for
//   printing to std in and out. Maybe implementing just that one interupt would be cool.

static command_line_args g_cmdLineArgs;

void printRegisterState(EmulationContext& ctx) {
    fmt::print("Final Registers:\n");

    auto reg = ctx.registers[i32(RegisterType::AX)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::BX)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::CX)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DX)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SP)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::BP)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SI)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DI)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);

    fmt::print("\n");

    reg = ctx.registers[i32(RegisterType::ES)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::SS)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::DS)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    reg = ctx.registers[i32(RegisterType::CS)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);

    fmt::print("\n");

    reg = ctx.registers[i32(RegisterType::IP)];
    fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);

    reg = ctx.registers[i32(RegisterType::FLAGS)];
    {
        char flagsBuf[BUFFER_SIZE_FLAGS] = {};
        flagsToCptr(Flags(reg.value), flagsBuf);
        fmt::print("\t{}: {} ({})\n", regTypeToCptr(reg.type), flagsBuf, reg.value);
    }
}

i32 main(i32 argc, char const** argv) {
    g_cmdLineArgs = initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666), "Failed to read file");

    DecodingContext ctx = {};
    switch (g_cmdLineArgs.immValuesFmt) {
        case 1: ctx.options = DecodingOpts(ctx.options | DEC_OP_IMMEDIATE_AS_HEX); break;
        case 2: ctx.options = DecodingOpts(ctx.options | DEC_OP_IMMEDIATE_AS_SIGNED); break;
        case 3: ctx.options = DecodingOpts(ctx.options | DEC_OP_IMMEDIATE_AS_UNSIGNED); break;
    }

    decodeAsm8086(binaryData, ctx);
    core::str_builder<> sb;
    encodeAsm8086(sb, ctx);

    if (!g_cmdLineArgs.execFlag || g_cmdLineArgs.verboseFlag) {
        fmt::print("bits 16\n\n");
        fmt::print("{}", sb.view().buff);
    }

    sb.clear();

    if (g_cmdLineArgs.execFlag) {
        EmulationContext emuCtx = createEmulationCtx(core::move(ctx.instructions));
        emuCtx.__verbosecity_buff = core::move(sb);
        if (g_cmdLineArgs.verboseFlag) {
            emuCtx.emuOpts = EmulationOpts(emuCtx.emuOpts | EmulationOpts::EMU_OPT_VERBOSE);
        }
        if (g_cmdLineArgs.verboseFlag) fmt::print("\n");
        emulate(emuCtx);
        if (g_cmdLineArgs.verboseFlag) fmt::print("\n");
        printRegisterState(emuCtx);
    }

    return 0;
}
