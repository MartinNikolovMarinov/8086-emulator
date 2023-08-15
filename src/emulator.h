#include "init_core.h"
#include "opcode.h"

// FIXME: Error handling should not allow for assertion crashes, or panics. Leaving this for last of course.

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
    Memory_Register, // FIXME: This might be switched around with Register_Memory ?
    Memory_Immediate,

    Register_Register,
    Register_Memory,
    Register_Immediate,

    Accumulator_Memory, // FIXME: This might be switched around with Memory_Accumulator ?
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
    core::arr<Instruction> instructions;
    core::arr<JmpLabel> jmpLabels;
};

void decodeAsm8086(core::arr<u8>& bytes, DecodingContext& ctx);
void encodeAsm8086(core::str_builder<>& asmOut, const DecodingContext& ctx);

enum struct RegisterType : u8 {
    AX, // Accumulator (AX) - Used in arithmetic operations.
    CX, // Counter (CX) - Used as a counter in string and loop operations.
    DX, // Data (DX) - Used in arithmetic operations.
    BX, // Base (BX) - Used as a pointer to data.
    SP, // Stack Pointer (SP) - Points to the top of the stack.
    BP, // Base Pointer (BP) - Used to point to data on the stack.
    SI, // Source Index (SI) - Used as a pointer to a source.
    DI, // Destination Index (DI) - Used as a pointer to a destination.

    ES, // Extra segment (ES) - General purpose segment register.
    CS, // Code segment (CS) - Points to the segment containing the current execution code.
    SS, // Stack segment (SS) - Points to the segment where the stack is maintained.
    DS, // Data segment (DS) - Points to the segment where variables are stored.

    IP, // Instruction Pointer (IP) - Contains the address of the next instruction to be executed.
    FLAGS, // Flags (FLAGS) - Contains the current state of the processor.
    SENTINEL
};

const char* regTypeToCptr(const RegisterType& r);

enum Flags : u16 {
    CPU_FLAG_NONE           = 0x0000,

    // CF (Carry Flag): Set if the last arithmetic operation carried (for addition) or borrowed (for subtraction) beyond
    // the most significant bit.
    CPU_FLAG_CARRY_FLAG     = 0x0001,

    // PF (Parity Flag): Set if the least significant byte of the result contains an even number of 1 bits.
    CPU_FLAG_PARITY_FLAG    = 0x0004,

    // AF (Auxiliary Carry Flag): Set if there was a carry from or borrow to bits 3 and 4 during the last arithmetic
    // operation.
    CPU_FLAG_AUX_CARRY_FLAG = 0x0010,

    // ZF (Zero Flag): Set if the result of the operation is 0.
    CPU_FLAG_ZERO_FLAG      = 0x0040,

    // SF (Sign Flag): Set if the most significant bit (sign bit) of the result is set.
    CPU_FLAG_SIGN_FLAG      = 0x0080,

    // OF (Overflow Flag): Set if there's an overflow in a signed arithmetic operation.
    CPU_FLAG_OVERFLOW_FLAG  = 0x0800,
};

static constexpr addr_size BUFFER_SIZE_FLAGS = 16;
char* flagsToCptr(Flags f, char* buffer);

struct Register {
    RegisterType type;
    u16 value;
};

enum EmulationOpts : u32 {
    EMU_OPT_NONE = 0,
    EMU_OPT_VERBOSE = 1 << 0,
};

struct EmulationContext {
    constexpr static addr_size MEMORY_SIZE = core::MEGABYTE;
    EmulationOpts emuOpts = EMU_OPT_NONE;
    DecodingOpts decodingOpts = DEC_OP_NONE;
    core::arr<Instruction> instructions;
    Register registers[i32(RegisterType::SENTINEL)];
    u8 memory[MEMORY_SIZE];

    core::str_builder<> __verbosecity_buff;
};

EmulationContext createEmulationCtx(core::arr<Instruction>&& instructions, EmulationOpts options = EMU_OPT_NONE);

void emulate(EmulationContext& ctx);

} // namespace asm8086

