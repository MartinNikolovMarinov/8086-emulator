#include <init_core.h>
#include <emulator.h>

static constexpr const char* pathToDecoderTests = PATH_TO_DATA"/test_files_for_decoding";
static constexpr const char* pathToNasmOutput = PATH_TO_BINARY"/current_tested.asm.o";

void runDecoderTests() {
    core::arr<u8> currentAsmFileBytes;
    core::arr<u8> currentTestedBinDataBytes;
    core::str_builder<> asmFileSb;
    core::str_builder<> commandSb;
    core::str_builder<> decodeOutputSb;

    core::os_dir_walk(pathToDecoderTests, [&](const core::dir_entry& de) -> bool {
        if (!de.isDir()) {
            defer {
                currentAsmFileBytes.clear();
                currentTestedBinDataBytes.clear();
                asmFileSb.clear();
                commandSb.clear();
                decodeOutputSb.clear();
            };

            // Read the assembly file:
            asmFileSb.append(pathToDecoderTests);
            asmFileSb.append("/");
            asmFileSb.append(de.name);

            Expect(core::os_read_entire_file(asmFileSb.view().data(), currentAsmFileBytes));

            // Compile the assembly file with nasm.
            {
                commandSb.append("nasm ");
                commandSb.append(asmFileSb.view().data());
                commandSb.append(" -o ");
                commandSb.append(pathToNasmOutput);

                auto sbView = commandSb.view();
                auto res = core::os_system(sbView.data());
                if (res.has_err()) {
                    fmt::print(stderr,
                                "Failed to execute system with \"{}\"; reason: {}\n",
                                sbView.data(),
                                core::os_get_err_cptr(res.err()));
                    Assert(false, "Call to system failed.");
                    return false;
                }

                i32 commandExitCode = res.value();
                if (commandExitCode != 0) {
                    fmt::print(stderr, "Nasm command exited with status code {}\n", commandExitCode);
                    Assert(false, "Nasm command failed.")
                    return false;
                }
            }

            Expect(core::os_read_entire_file(pathToNasmOutput, currentTestedBinDataBytes));

            // Run the decoder:
            {
                asm8086::DecodingContext ctx;
                asm8086::decodeAsm8086(currentTestedBinDataBytes, ctx);
                asm8086::encodeAsm8086(decodeOutputSb, ctx);

                // Compare the results:
                auto sbView = decodeOutputSb.view();
                bool diffFound = false;
                for (addr_size i = 0; i < currentAsmFileBytes.len(); ++i) {
                    auto& expected = currentAsmFileBytes[i];
                    auto& actual = sbView.data()[i];
                    if (expected != actual) {
                        diffFound = true;
                        break;
                    }
                }

                if (diffFound) {
                    fmt::print(stderr, "Test for file {} failed.\n", de.name);
                }
                else {
                    fmt::print("Test for file {} passed.\n", de.name);
                }
            }
        }

        return true;
    });
}

i32 main() {
    // std::cout << "\t[TEST " << "№ " << g_testCount << " RUNNING] " << ANSI_BOLD(#test) << '\n';
    fmt::print("Running tests...\n");
    fmt::print("Starting decoder tests...\n");
    runDecoderTests();
    return 0;
}
