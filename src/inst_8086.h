#include "init_core.h"
#include "opcode.h"
#include "mod.h"

namespace asm8086 {

enum struct InstType : u8 {
    MOV,
    ADD,
    SUB,

    SENTINEL
};

// Register/memory to/from register.
//
// [byte1] - opcode:6, d|v:1, w:1
// [byte2] - mod:2, reg:3, rm:3
// [byte3] - (disp:8)
// [byte4] - (disp:8)
struct RegMemToFromReg {
    union
    {
        u8 d : 1;
        u8 v : 1;
    };

    u8 w : 1;
    u8 reg : 3;
    u8 rm : 3;
    Mod mod : 2;
    u8 disp[2];
};

// Immediate to register. Very specific for the MOV instruction.
//
// [byte1] - opcode:4, w:1, reg:3
// [byte2] - data:8
// [byte3] - (data:8)
struct ImmToReg {
    u8 w : 1;
    u8 reg : 3;
    u8 data[2];
};

// Immediate to register or memory.
//
// [byte1] - opcode:7, w:1
// [byte2] - mod:2, reg:3 (constant), rm:3
// [byte3] - (disp:8)
// [byte4] - (disp:8)
// [byte5] - data:8
// [byte6] - (data:8)
struct ImmToRegMem {
    u8 w : 1;
    u8 reg : 3;
    u8 rm : 3;
    Mod mod : 2;
    u8 disp[2];
    u8 data[2];
};

// Memory/accumulator to accumulator/memory.
//
// [byte1] - opcode:6, d:1, w:1
// [byte2] - addr-lo:8
// [byte3] - addr-hi:8
struct MemAccToAccMem {
    u8 d : 1;
    u8 w : 1;
    u8 addr[2];
};

struct Inst {
    InstType type;
    Opcode opcode;
    union {
        RegMemToFromReg regMemToFromReg;
        ImmToReg        immToReg;
        ImmToRegMem     immToRegMem;
        MemAccToAccMem  memAccToAccMem;
    };
};

Inst decodeAsm8086(core::arr<u8>& bytes, i32& idx);
void encodeAsm8086(core::str_builder<>& sb, const Inst& inst);

} // namespace asm8086

