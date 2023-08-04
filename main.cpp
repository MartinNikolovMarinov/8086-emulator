#include "src/init_core.h"
#include "src/emulator.h"

#include <fcntl.h>

using namespace asm8086;

static command_line_args g_cmdLineArgs;
static constexpr ptr_size g_memorySize = core::MEGABYTE;
static constexpr ptr_size g_segmentSize = g_memorySize / (64 * core::KILOBYTE) + 1;
[[maybe_unused]] static u8 g_memory[g_memorySize];

i32 main(i32 argc, char const** argv) {
    g_cmdLineArgs = initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666), "Failed to read file");
    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);
    core::str_builder<> sb;
    encodeAsm8086(sb, ctx);

    if (!g_cmdLineArgs.execFlag || g_cmdLineArgs.verboseFlag) {
        fmt::print("bits 16\n\n");
        fmt::print("{}", sb.view().buff);
    }

    return 0;
}
