#include "init_core.h"
#include "opcode.h"

// FIXME: Error handling should not allow for assertion crashes, or panics. Leaving this for last of course.

namespace asm8086 {

enum struct Mod : u8 {
    MEMORY_NO_DISPLACEMENT               = 0b00,
    MEMORY_8_BIT_DISPLACEMENT            = 0b01,
    MEMORY_16_BIT_DISPLACEMENT           = 0b10,
    REGISTER_TO_REGISTER_NO_DISPLACEMENT = 0b11,
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
    addr_off byteOffset;
    addr_off labelIdx;
};

struct DecodingContext {
    addr_size idx = 0;
    core::arr<Instruction> instructions;
    core::arr<JmpLabel> jmpLabels;
};

void decodeAsm8086(core::arr<u8>& bytes, DecodingContext& ctx);
void encodeAsm8086(core::str_builder<>& asmOut, const DecodingContext& ctx);

enum struct RegisterType : u8 {
    AX,
    CX,
    DX,
    BX,
    SP,
    BP,
    SI,
    DI,

    CS, // Code segment (CS) - Points to the segment containing the current execution code.
    DS, // Data segment (DS) - Points to the segment where variables are stored.
    SS, // Stack segment (SS) - Points to the segment where the stack is maintained.
    ES, // Extra segment (ES) - General purpose segment register.

    IP,
    FLAGS,
    SENTINEL
};

const char* regTypeToCptr(const RegisterType& r);

struct Register {
    u16 value;
    RegisterType type;
};

enum EmulationOptionFlags : u32 {
    EMU_FLAG_NONE    = 0,
    EMU_FLAG_VERBOSE = 1 << 0,
};

enum Flags : u16 {
    // CF (Carry Flag): Set if the last arithmetic operation carried (for addition) or borrowed (for subtraction) beyond
    // the most significant bit.
    SF_CARRY_FLAG     = 0x0001,

    // PF (Parity Flag): Set if the least significant byte of the result contains an even number of 1 bits.
    SF_PARITY_FLAG    = 0x0004,

    // AF (Auxiliary Carry Flag): Set if there was a carry from or borrow to bits 3 and 4 during the last arithmetic
    // operation.
    SF_AUX_CARRY_FLAG = 0x0010,

    // ZF (Zero Flag): Set if the result of the operation is 0.
    SF_ZERO_FLAG      = 0x0040,

    // SF (Sign Flag): Set if the most significant bit (sign bit) of the result is set.
    SF_SIGN_FLAG      = 0x0080,

    // OF (Overflow Flag): Set if there's an overflow in a signed arithmetic operation.
    SF_OVERFLOW_FLAG  = 0x0800,
};

struct EmulationContext {
    constexpr static addr_size MEMORY_SIZE = core::MEGABYTE;
    EmulationOptionFlags options;
    core::arr<Instruction> instructions;
    Register registers[i32(RegisterType::SENTINEL)];
    u8 memory[MEMORY_SIZE];
};

EmulationContext createEmulationCtx(core::arr<Instruction>&& instructions, EmulationOptionFlags options = EMU_FLAG_NONE);

void emulate(EmulationContext& ctx);

} // namespace asm8086

