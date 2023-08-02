#include "src/inst_8086.h"

#include <fmt/format.h>
#include <fcntl.h>

using namespace asm8086;

void debugPrintInst(const Inst& inst) {
    core::str_builder sb;
    encodeAsm8086(sb, inst);
    fmt::print("{}\n", sb.view().buff);
}

#define DEBUG_PRINT_INSTRUCTIONS 1

i32 main(i32 argc, char const** argv) {
    initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666), "Failed to read file");
    core::str_builder encodedStream;
    encodedStream.append("bits 16\n\n"); // TODO: Hardcoding 16 bit mode, but this should be detected from the binary.
    i32 idx = 0;
    i32 instCount = 0;
    while (idx < binaryData.len()) {
        auto inst = decodeAsm8086(binaryData, idx);

#if defined(DEBUG_PRINT_INSTRUCTIONS) && DEBUG_PRINT_INSTRUCTIONS == 1
        debugPrintInst(inst);
#endif

        // encodeInst(encodedStream, inst);
        // encodedStream.append("\n");
        instCount++;
    }

#if !defined(DEBUG_PRINT_INSTRUCTIONS) || DEBUG_PRINT_INSTRUCTIONS == 0
    fmt::print("{}", encodedStream.view().buff);
#endif
    return 0;
}
