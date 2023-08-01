#include "init_core.h"
#include "opcode.h"
#include "mov.h"

enum struct InstType : u8 {
    MOV,
    ADD,
    SUB,

    SENTINEL
};

const char* instTypeToCptr(InstType t);

// [byte1] - opcode:6, d:1, w:1
// [byte2] - mod:2, reg:3, rm:3
// [byte3] - (disp:8)
// [byte4] - (disp:8)
struct Inst_v1 {
    InstType type : 4;
    u8 d : 1;
    u8 w : 1;
    u8 reg : 3;
    u8 rm : 3;
    Mod mod : 2;
    u16 disp : 16;
};

void encodeInst(core::str_builder<>& sb, const Inst_v1& inst);

// [byte1] - opcode:6, s|v|d:1, w:1
// [byte2] - mod:2, reg:3 (constant), rm:3
// [byte3] - (disp:8)
// [byte4] - (disp:8)
// [byte5] - (data:8)
// [byte6] - (data:8)
struct Inst_v2 {
    InstType type : 4;
    union {
        u8 s : 1;
        u8 v : 1;
    };
    u8 w : 1;
    Mod mod : 2;
    u8 rm : 3;
    u16 disp : 16;
    u16 data : 16;
};

void encodeInst(core::str_builder<>& sb, const Inst_v2& inst);

struct Inst8086 {
    Opcode opcode;
    union {
        Inst_v1    movRegOrMemToOrFromReg;
        Inst_v2    movImmToRegOrMem;
        MovInst_v3 movImmToReg;
        MovInst_v4 movMemToAcc;
        Inst_v1    addRegOrMemWithRegToEdit;
        Inst_v2    addImmToRegOrMem;
    };
};

void     encodeInst(core::str_builder<>& sb, const Inst8086& inst);
Inst8086 decodeInst(core::arr<u8>& bytes, i32& idx);
