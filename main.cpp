#include <init_core.h>
#include <fmt/format.h>

i32 main(i32, char const**) {
    initCore();
    u8 input[] = {0x89, 0xD9}; // mov cx, bx

    u8 opcode;
    u8 d;
    u8 w;
    u8 mod;
    u8 reg;
    u8 rm;

    opcode = input[0] >> 2;
    d = (input[0] >> 1) & 1;
    w = input[0] & 1;

    mod = input[1] >> 6;
    reg = (input[1] >> 3) & 3;
    rm = input[1] & 3;

    return 0;
}
