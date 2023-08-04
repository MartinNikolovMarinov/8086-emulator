#include "init_core.h"
#include "opcode.h"

namespace asm8086 {

enum Mod : u8 {
    MOD_MEMORY_NO_DISPLACEMENT               = 0b00,
    MOD_MEMORY_8_BIT_DISPLACEMENT            = 0b01,
    MOD_MEMORY_16_BIT_DISPLACEMENT           = 0b10,
    MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT = 0b11,
};

enum struct InstType : u8 {
    UNKNOWN,
    MOV,
    ADD,
    SUB,
    CMP,
    JNEZ,

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

    ShortLabel,

    SegReg_Register16,
    SetReg_Memory16,
    Register16_SegReg,
    Memory_SegReg,
};

struct Instruction {
    // byte 1
    Opcode opcode; // is variable length
    u8 d;
    u8 s;
    u8 w;

    // byte 2
    Mod mod;
    u8 reg;
    u8 rm;

    // byte 3
    u8 disp[2];

    // byte 4
    u8 data[2];

    // Decoded fields.
    InstType type;
    u8 byteCount;
    Operands operands;
};

struct DecodingContext {
    ptr_size idx = 0;
    core::arr<Instruction> instructions;
    core::arr<i64> labelAddrs;
};

void decodeAsm8086(core::arr<u8>& bytes, DecodingContext& ctx);
void encodeAsm8086(core::str_builder<>& sb, const DecodingContext& ctx);

} // namespace asm8086

