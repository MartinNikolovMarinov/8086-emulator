#include "init_core.h"
#include "opcode.h"
#include "mov.h"

struct Inst8086 {
    Opcode opcode;
    union {
        MovInst_v1 movRegOrMemToOrFromReg;
        MovInst_v2 movImmToRegOrMem;
        MovInst_v3 movImmToReg;
        MovInst_v4 movMemToAcc;
    };

    void encode(core::str_builder<>& sb) const;
};

Inst8086 decodeInst(core::arr<u8>& bytes, i32& idx);
