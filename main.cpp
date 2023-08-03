#include "src/inst_8086.h"

#include <fmt/format.h>
#include <fcntl.h>

using namespace asm8086;

static constexpr ptr_size g_memorySize = core::MEGABYTE;
static constexpr ptr_size g_segmentSize = g_memorySize / (64 * core::KILOBYTE) + 1;
[[maybe_unused]] static u8 g_memory[g_memorySize];

i32 main(i32 argc, char const** argv) {
    initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666), "Failed to read file");
    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);
    core::str_builder<> sb;
    encodeAsm8086(sb, ctx);
    fmt::print("{}", sb.view().buff);

    return 0;
}
