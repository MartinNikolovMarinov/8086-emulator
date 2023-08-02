#include "src/inst_8086.h"

#include <fmt/format.h>
#include <fcntl.h>

using namespace asm8086;

#define DEBUG_PRINT_INSTRUCTIONS 1

#if defined(DEBUG_PRINT_INSTRUCTIONS) && DEBUG_PRINT_INSTRUCTIONS == 1

void debugPrintInst(const Inst& inst) {
    core::str_builder sb;
    encodeAsm8086(sb, inst);
    fmt::print("{}\n", sb.view().buff);
}

#endif

i32 main(i32 argc, char const** argv) {
    initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666), "Failed to read file");
    core::str_builder encodedStream;
    encodedStream.append("bits 16\n\n");
    i32 idx = 0;
    i32 instCount = 0;
    while (idx < binaryData.len()) {
        auto inst = decodeAsm8086(binaryData, idx);
#if defined(DEBUG_PRINT_INSTRUCTIONS) && DEBUG_PRINT_INSTRUCTIONS == 1
        debugPrintInst(inst);
#else
        encodeAsm8086(encodedStream, inst);
        encodedStream.append("\n");
#endif
        instCount++;
    }

#if !defined(DEBUG_PRINT_INSTRUCTIONS) || DEBUG_PRINT_INSTRUCTIONS == 0
    fmt::print("{}", encodedStream.view().buff);
#endif
    return 0;
}
