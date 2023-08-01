#include "src/inst_8086.h"

#include <fmt/format.h>
#include <fcntl.h>

i32 main(i32 argc, char const** argv) {
    initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666), "Failed to read file");
    core::str_builder encodedStream;
    encodedStream.append("bits 16\n\n"); // TODO: Hardcoding 16 bit mode, but this should be detected from the binary.
    i32 idx = 0;
    i32 instCount = 0;
    while (idx < binaryData.len()) {
        auto inst = decodeInst(binaryData, idx);
        inst.encode(encodedStream);
        encodedStream.append("\n");
        instCount++;
    }

    fmt::print("{}", encodedStream.view().buff);
    return 0;
}
