#include "src/init_core.h"
#include "src/emulator.h"

#include <fcntl.h>

using namespace asm8086;

static command_line_args g_cmdLineArgs;
static constexpr addr_size g_memorySize = core::MEGABYTE;
static constexpr addr_size g_segmentSize = g_memorySize / (64 * core::KILOBYTE) + 1;
[[maybe_unused]] static u8 g_memory[g_memorySize];

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
