#include <init_core.h>
#include <fmt/format.h>

#include <fcntl.h>
#include <string>

enum Opcodes {
    MOV_REG_TO_REG = 0b100010,
    SENTINEL
};

struct MovInst {
    u8 byte1;
    u8 byte2;

    u8 opcode() const { return byte1 >> 2; }
    u8 d() const { return (byte1 >> 1) & 0b1; }
    u8 w() const { return byte1 & 0b1; }
    u8 mod() const { return byte2 >> 6; }
    u8 reg() const { return (byte2 >> 3) & 0b111; }
    u8 rm() const { return byte2 & 0b111; }
};

bool decodeRegs(u32 r, u32 w, std::string& out) {
    switch (r) {
        case 0b000: out += (w == 0) ? "al" : "ax"; break;
        case 0b001: out += (w == 0) ? "cl" : "cx"; break;
        case 0b010: out += (w == 0) ? "dl" : "dx"; break;
        case 0b011: out += (w == 0) ? "bl" : "bx"; break;
        case 0b100: out += (w == 0) ? "ah" : "sp"; break;
        case 0b101: out += (w == 0) ? "ch" : "bp"; break;
        case 0b110: out += (w == 0) ? "dh" : "si"; break;
        case 0b111: out += (w == 0) ? "bh" : "di"; break;
        default: return false;
    }
    return true;
}

bool decode(MovInst& inst, std::string& out) {
    out += "mov ";
    if (bool ok = decodeRegs(inst.rm(), inst.w(), out); !ok) return false;
    out += ", ";
    if (inst.mod() == 0b11) {
        if (bool ok = decodeRegs(inst.reg(), inst.w(), out); !ok) return false;
    } else {
        Assert(false, "not implemented yet");
    }
    out += "\n";
    return true;
}

i32 main(i32 argc, char const** argv) {
    initCore();

    if (argc != 2) {
        fmt::print("Program requires exactly one argument as a file name to the executable\n");
        return 1;
    }

    char const* fileName = argv[1];
    auto binaryData = ValueOrDie(core::readFull(fileName, O_RDONLY, 0666));
    std::string out = "bits 16\n";
    for (i32 i = 0; i < binaryData.len(); i+=2) {
        MovInst mov;
        mov.byte1 = binaryData[i];
        mov.byte2 = binaryData[i+1];
        Assert(decode(mov, out));
    }

    fmt::print("{}\n", out.data());
    return 0;
}
