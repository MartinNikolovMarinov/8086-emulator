#include "init_core.h"
#include "opcode.h"

// FIXME: Error handling should not allow for assertion crashes, or panics. Leaving this for last of course.

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
    JE,  JZ,   // These are synonyms
    JL,  JNGE, // These are synonyms
    JLE, JNG,  // These are synonyms
    JB,  JNAE, // These are synonyms
    JBE, JNA,  // These are synonyms
    JP,  JPE,  // These are synonyms
    JO,
    JS,
    JNE,  JNZ, // These are synonyms
    JNL,  JGE, // These are synonyms
    JNLE, JG,  // These are synonyms
    JNB,  JAE, // These are synonyms
    JNBE, JA,  // These are synonyms
    JNP,  JPO, // These are synonyms
    JNO,
    JNS,
    LOOP,
    LOOPE, LOOPZ,   // These are synonyms
    LOOPNE, LOOPNZ, // These are synonyms
    JCXZ,

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

struct JmpLabel {
    ptr_size byteOffset;
    ptr_size labelIdx;
};

struct DecodingContext {
    ptr_size idx = 0;
    core::arr<Instruction> instructions;
    core::arr<JmpLabel> jmpLabels;
};

void decodeAsm8086(core::arr<u8>& bytes, DecodingContext& ctx);
void encodeAsm8086(core::str_builder<>& asmOut, const DecodingContext& ctx);

} // namespace asm8086

