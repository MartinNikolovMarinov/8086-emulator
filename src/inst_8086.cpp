#include "inst_8086.h"

void Inst8086::encode(core::str_builder<>& sb) const {
    switch (opcode) {
        case MOV_REG_OR_MEM_TO_OR_FROM_REG: movRegOrMemToOrFromReg.encode(sb); break;
        case MOV_IMM_TO_REG:                movImmToReg.encode(sb);            break;
        case MOV_IMM_TO_REG_OR_MEM:         movImmToRegOrMem.encode(sb);       break;
        case MOV_MEM_TO_ACC:                movMemToAcc.encode(sb);            break;
        default:                            Panic(false, "Opcode unsupported or invalid");
    }
}

Inst8086 decodeInst(core::arr<u8>& bytes, i32& idx) {
    auto o   = [](u8 b) -> Opcode { return opcodeDecode(b); };
    auto w   = [](u8 b) -> bool   { return (b >> 0) & 0b1; };
    auto d   = [](u8 b) -> bool   { return (b >> 1) & 0b1; };
    auto mod = [](u8 b) -> Mod    { return Mod((b >> 6) & 0b11); };
    auto reg = [](u8 b) -> u8     { return (b >> 3) & 0b111; };
    auto rm  = [](u8 b) -> u8     { return (b >> 0) & 0b111; };

    auto data = [](auto& bytes, i32& idx, u8 w) -> i16 {
        if (w == 0) {
            u8 byte2 = bytes[idx++];
            return i16(i8(byte2)); // Be very careful not to drop the sign!
        }
        else {
            u8 byte2 = bytes[idx++];
            u8 byte3 = bytes[idx++];
            return (i16(byte3) << 8) | i16(byte2);
        }
    };

    auto disp = [](auto& bytes, i32& idx, u8 mod) -> i16 {
        if (mod == MOD_MEMORY_8_BIT_DISPLACEMENT) {
            u8 byte3 = bytes[idx++];
            return u16(i8(byte3));
        }
        else if (mod == MOD_MEMORY_16_BIT_DISPLACEMENT) {
            u8 byte3 = bytes[idx++];
            u8 byte4 = bytes[idx++];
            return (i16(byte4) << 8) | i16(byte3);
        }
        return 0;
    };

    auto addr = [](auto& bytes, i32& idx) -> u16 {
        u8 byte2 = bytes[idx++];
        u8 byte3 = bytes[idx++];
        return u16(byte3 << 8) | u16(byte2);
    };

    u8 byte1 = bytes[idx++];
    Inst8086 res = {};
    res.opcode = o(byte1);

    switch (res.opcode) {
        case MOV_IMM_TO_REG:
        {
            // For this command w and reg are in the first byte.
            MovInst_v3 inst = {};
            inst.w = (byte1 >> 3) & 0b1;
            inst.reg = byte1 & 0b111;
            inst.data = data(bytes, idx, inst.w);
            res.movImmToReg = inst;
            return res;
        }
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:
        {
            u8 byte2 = bytes[idx++];
            MovInst_v1 inst = {};
            inst.d = d(byte1);
            inst.w = w(byte1);
            inst.mod = mod(byte2);
            inst.reg = reg(byte2);
            inst.rm = rm(byte2);

            bool isDirectAddressing = (inst.mod == MOD_MEMORY_NO_DISPLACEMENT && inst.rm == 0b110);
            if (isDirectAddressing) {
                // When Direct addressing use the displacement as the address data.
                inst.disp = data(bytes, idx, inst.w);
            }
            else {
                inst.disp = disp(bytes, idx, inst.mod);
            }

            res.movRegOrMemToOrFromReg = inst;
            return res;
        }
        case MOV_MEM_TO_ACC: [[fallthrough]];
        case MOV_ACC_TO_MEM:
        {
            MovInst_v4 inst = {};
            inst.w = w(byte1);
            inst.addr = addr(bytes, idx);
            inst.d = d(byte1);
            res.movMemToAcc = inst;
            return res;
        }
        case MOV_IMM_TO_REG_OR_MEM:
        {
            u8 byte2 = bytes[idx++];
            MovInst_v2 inst = {};
            inst.w = w(byte1);
            inst.mod = mod(byte2);
            inst.rm = rm(byte2);
            inst.disp = disp(bytes, idx, inst.mod);
            inst.data = data(bytes, idx, inst.w);
            res.movImmToRegOrMem = inst;
            return res;
        }
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:
            Assert(false, "Not implemented");
            return res;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:
            Assert(false, "Not implemented");
            return res;
    }

    return res;
}
