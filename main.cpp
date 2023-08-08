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
    for (i32 i = 0; i < i32(RegisterType::SENTINEL); ++i) {
        auto reg = ctx.registers[i];
        if (reg.type == RegisterType::CS || reg.type == RegisterType::IP) {
            fmt::print("\n");
        }
        fmt::print("\t{}: {:#06x} ({})\n", regTypeToCptr(reg.type), reg.value, reg.value);
    }
}

i32 main(i32 argc, char const** argv) {
    g_cmdLineArgs = initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666), "Failed to read file");
    DecodingContext ctx = {};
    ctx.options = DecodingOptionFlags(ctx.options | DecodingOptionFlags::DE_FLAG_IMMEDIATE_AS_HEX);
    decodeAsm8086(binaryData, ctx);
    core::str_builder<> sb;
    encodeAsm8086(sb, ctx);

    if (!g_cmdLineArgs.execFlag) {
        fmt::print("bits 16\n\n");
        fmt::print("{}", sb.view().buff);
    }
    if (g_cmdLineArgs.execFlag) {
        EmulationContext emuCtx = createEmulationCtx(core::move(ctx.instructions));
        if (g_cmdLineArgs.verboseFlag) {
            emuCtx.options = EmulationOptionFlags(emuCtx.options | EmulationOptionFlags::EMU_FLAG_VERBOSE);
        }
        emulate(emuCtx);
        printRegisterState(emuCtx);
    }

    return 0;
}
