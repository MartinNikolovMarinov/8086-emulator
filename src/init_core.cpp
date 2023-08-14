#include "init_core.h"

command_line_args g_cmdLineArgs;

void* std_allocator_static::alloc(addr_size size) noexcept {
    return g_stdAlloc.alloc(size);
}

void std_allocator_static::free(void* ptr) noexcept {
    g_stdAlloc.free(ptr);
}

addr_size std_allocator_static::used_mem() noexcept {
    return g_stdAlloc.used_mem();
}

const char* std_allocator_static::allocator_name() noexcept {
    return g_stdAlloc.allocator_name();
}

void printUsage() {
    fmt::print(fmt::emphasis::bold, "Usage:\n");
    fmt::print("  -f (required)       the binary file to use.\n");
    fmt::print("  --exec              emulate the execution.\n");
    fmt::print("  --verbose           print verbose information.\n");
    fmt::print("  --imm-values-fmt    the format to use when printing immediate values.\n");
    fmt::print("                      0 - is the default.\n");
    fmt::print("                      1 - use hex format.\n");
    fmt::print("                      2 - use signed format.\n");
    fmt::print("                      3 - use unsigned format.\n");
}

command_line_args initCore(i32 argc, const char** argv) {
    core::set_global_assert_handler([](const char* failedExpr, const char* file, i32 line, const char* errMsg) {
        constexpr u32 stackFramesToSkip = 3;
        std::string trace = core::stacktrace(200, stackFramesToSkip);
        fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                   "[ASSERTION] [EXPR]: {}\n[FILE]: {}:{}\n[MSG]: {}\n",
                    failedExpr, file, line, errMsg);
        fmt::print(fmt::emphasis::bold, "[TRACE]:\n{}\n", trace);
        throw std::runtime_error("Assertion failed!");
        return false;
    });

    core::rnd_init();

    // Parse the command line arguments.
    command_line_args cmdLineArgs = {};
    bool argsAreOk = false;
    if (argc > 1) {
        core::flag_parser parser;
        parser.allowUnknownFlags = true;
        parser.flag(&cmdLineArgs.fileName, "f", true);
        parser.flag(&cmdLineArgs.execFlag, "exec", false);
        parser.flag(&cmdLineArgs.verboseFlag, "verbose", false);
        parser.flag(&cmdLineArgs.immValuesFmt, "imm-values-fmt", false, [](void* a) -> bool {
            u32 v = *(u32*)a;
            return (v <= 3);
        });

        auto res = parser.parse(argc - 1, argv + 1);
        argsAreOk = !res.has_err();
    }

    if (!argsAreOk) {
        fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Invalid arguments.\n\n");
        printUsage();
        core::os_exit(-1);
    }

    return cmdLineArgs;
}
