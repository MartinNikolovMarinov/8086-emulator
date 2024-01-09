#pragma once

#include <init_core.h>
#include <opcodes.h>

namespace asm8086 {

enum struct Mod : u8 {
    MEMORY_NO_DISPLACEMENT               = 0b00,
    MEMORY_8_BIT_DISPLACEMENT            = 0b01,
    MEMORY_16_BIT_DISPLACEMENT           = 0b10,
    REGISTER_TO_REGISTER_NO_DISPLACEMENT = 0b11,

    NONE_SENTINEL
};

const char* modeToCptr(Mod mod);

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

const char* instTypeToCptr(InstType t);

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
    SegReg_Memory16,
    Register16_SegReg,
    Memory_SegReg,

    SENTINEL
};

const char* operandsToCptr(Operands o);

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


// This should not be used for decoding, it only prints information about the instruction.
static constexpr addr_size BUFFER_SIZE_INST_INFO_OUT = 512;
char* instructionToInfoCptr(const Instruction& inst, char* out);

struct JmpLabel {
    addr_off byteOffset;
    addr_off labelIdx;
};

enum DecodingOpts : u32 {
    DEC_OP_NONE = 0,
    DEC_OP_IMMEDIATE_AS_HEX = 1 << 1,
    DEC_OP_IMMEDIATE_AS_SIGNED = 1 << 2,
    DEC_OP_IMMEDIATE_AS_UNSIGNED = 1 << 3,
};

struct DecodingContext {
    addr_size idx = 0;
    DecodingOpts options = DEC_OP_NONE;
    core::Arr<Instruction> instructions;
    core::Arr<JmpLabel> jmpLabels;
};

void decodeAsm8086(core::Arr<u8>& bytes, DecodingContext& ctx);
void encodeAsm8086(core::StrBuilder<>& asmOut, const DecodingContext& ctx);

namespace detail {
    void encodeBasicInstruction(core::StrBuilder<>& sb, const Instruction& inst, DecodingOpts decodingOpts);
}

} // namespace asm8086
