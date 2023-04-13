#include <init_core.h>
#include <fmt/format.h>

struct Instruction {
    u8 byte1;
    u8 byte2;

    u8 opcode() const { return byte1 >> 2; }
    u8 d() const { return (byte1 >> 1) & 1; }
    u8 w() const { return byte1 & 1; }
    u8 mod() const { return byte2 >> 6; }
    u8 reg() const { return (byte2 >> 3) & 3; }
    u8 rm() const { return byte2 & 3; }
};

i32 main(i32, char const**) {
    initCore();
    u8 input[] = {0xD9, 0x89}; // mov cx, bx

    Instruction instruction;
    instruction.byte1 = input[0];
    instruction.byte2 = input[1];

    fmt::print("opcode: {}\n", instruction.opcode());
    fmt::print("d: {}\n", instruction.d());
    fmt::print("w: {}\n", instruction.w());
    fmt::print("mod: {}\n", instruction.mod());
    fmt::print("reg: {}\n", instruction.reg());
    fmt::print("rm: {}\n", instruction.rm());

    return 0;
}
