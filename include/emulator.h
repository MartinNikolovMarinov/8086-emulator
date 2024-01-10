#include <init_core.h>
#include <decoder.h>

namespace asm8086 {

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

constexpr static addr_size EMULATOR_MEMORY_SIZE = core::MEGABYTE;

struct EmulationContext {
    EmulationOpts emuOpts = EMU_OPT_NONE;
    DecodingOpts decodingOpts = DEC_OP_NONE;
    core::Arr<Instruction> instructions;
    Register registers[i32(RegisterType::SENTINEL)];
    u8* memory = nullptr;

    core::StrBuilder<> __verbosecity_buff;
};

EmulationContext createEmulationCtx(core::Arr<Instruction>&& instructions, EmulationOpts options = EMU_OPT_NONE);

void emulate(EmulationContext& ctx);

} // namespace asm8086
