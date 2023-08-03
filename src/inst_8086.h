#include "init_core.h"
#include "opcode.h"
#include "mod.h"

namespace asm8086 {

enum struct InstType : u8 {
    UNKNOWN,
    MOV,
    ADD,
    SUB,
    CMP,

    SENTINEL
};

enum struct Operands : u8 {
    None,
    Memory_Accumulator,
    Memory_Register,
    Memory_Immediate,

    Register_Register,
    Register_Memory,
    Register_Immediate,

    Accumulator_Memory,
    Accumulator_Immediate,

    SegReg_Register16,
    SetReg_Memory16,
    Register16_SegReg,
    Memory_SegReg,
};

struct Instruction {
    // byte 1
    Opcode opcode; // is variable length
    u8 d : 1;
    u8 s : 1;
    u8 w : 1;

    // byte 2
    Mod mod : 2;
    u8 reg : 3;
    u8 rm : 3;

    // byte 3
    u8 disp[2];

    // byte 4
    u8 data[2];

    // Decoded fields.
    InstType type;
    u8 byteCount;
    Operands operands;
};

Instruction decodeAsm8086(core::arr<u8>& bytes, i32& idx);
void encodeAsm8086(core::str_builder<>& sb, const Instruction& inst);

} // namespace asm8086

