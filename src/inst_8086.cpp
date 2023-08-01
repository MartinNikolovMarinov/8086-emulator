#include "inst_8086.h"
#include "utils.h"

const char* instTypeToCptr(InstType t) {
    switch (t) {
        case InstType::MOV: return "mov";
        case InstType::ADD: return "add";
        case InstType::SUB: return "sub";
        case InstType::SENTINEL: break;
    }
    return "unknown";
}

void encodeInst(core::str_builder<>& sb, const Inst_v1& inst) {
    auto type = inst.type;
    auto d = inst.d;
    auto w = inst.w;
    auto reg = inst.reg;
    auto rm = inst.rm;
    auto mod = inst.mod;
    auto disp = inst.disp;

    const char* cptrType = instTypeToCptr(type);
    sb.append(cptrType);
    sb.append(" ");

    if (mod == MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT) {
        if (d) {
            sb.append(regToCptr(reg, w));
            sb.append(", ");
            sb.append(regToCptr(rm, w));
        }
        else {
            sb.append(regToCptr(rm, w));
            sb.append(", ");
            sb.append(regToCptr(reg, w));
        }
    }
    else if (mod == MOD_MEMORY_NO_DISPLACEMENT) {
        bool isDirectAddressing = (rm == 0b110);

        if (d) {
            sb.append(regToCptr(reg, w));
            sb.append(", ");
            sb.append("[");
            if (isDirectAddressing) {
                appendIntToSb_AsImmediate(sb, disp);
            } else {
                sb.append(rmEffectiveAddrCalc(rm));
            }
            sb.append("]");
        }
        else {
            sb.append("[");
            if (isDirectAddressing) {
                appendIntToSb_AsImmediate(sb, disp);
            } else {
                sb.append(rmEffectiveAddrCalc(rm));
            }
            sb.append("]");
            sb.append(", ");
            sb.append(regToCptr(reg, w));
        }
    }
    else {
        // 8 or 16 bit displacement.
        if (d) {
            sb.append(regToCptr(reg, w));
            sb.append(", ");
            sb.append("[");
            sb.append(rmEffectiveAddrCalc(rm));
            if (disp != 0) appendIntToSb_AsDisp(sb, disp, mod);
            sb.append("]");
        }
        else {
            sb.append("[");
            sb.append(rmEffectiveAddrCalc(rm));
            if (disp != 0) appendIntToSb_AsDisp(sb, disp, mod);
            sb.append("]");
            sb.append(", ");
            sb.append(regToCptr(reg, w));
        }
    }
}

void encodeInst(core::str_builder<>& sb, const Inst_v2& inst) {
    auto type = inst.type;
    auto w = inst.w;
    auto mod = inst.mod;
    auto rm = inst.rm;
    auto disp = inst.disp;
    auto data = inst.data;

    const char* cptrType = instTypeToCptr(type);
    sb.append(cptrType);
    sb.append(" ");

    if (mod == MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT) {
        sb.append(regToCptr(rm, w));
        sb.append(", ");
        sb.append(w ? "word" : "byte");
        sb.append(" ");
        appendIntToSb_AsImmediate(sb, data);
    }
    else if (mod == MOD_MEMORY_NO_DISPLACEMENT) {
        sb.append("[");
        sb.append(rmEffectiveAddrCalc(rm));
        sb.append("]");
        sb.append(", ");
        sb.append(w ? "word" : "byte");
        sb.append(" ");
        appendIntToSb_AsImmediate(sb, data);
    }
    else {
        // 8 or 16 bit displacement.
        sb.append("[");
        sb.append(rmEffectiveAddrCalc(rm));
        appendIntToSb_AsDisp(sb, disp, mod);
        sb.append("]");
        sb.append(", ");
        sb.append(w ? "word" : "byte");
        sb.append(" ");
        appendIntToSb_AsImmediate(sb, data);
    }
}

void encodeInst(core::str_builder<>& sb, const Inst8086& inst) {
    switch (inst.opcode) {
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:   encodeInst(sb, inst.movRegOrMemToOrFromReg);   return;
        case MOV_IMM_TO_REG:                  inst.movImmToReg.encode(sb);                   return;
        case MOV_IMM_TO_REG_OR_MEM:           encodeInst(sb, inst.movImmToRegOrMem);         return;
        case MOV_ACC_TO_MEM:                                                                 [[fallthrough]];
        case MOV_MEM_TO_ACC:                  inst.movMemToAcc.encode(sb);                   return;

        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:                                               break; // TODO:
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:                                               break; // TODO:

        case ADD_REG_OR_MEM_WITH_REG_TO_EDIT: encodeInst(sb, inst.addRegOrMemWithRegToEdit); return;
        case ADD_IMM_TO_REG_OR_MEM:           encodeInst(sb, inst.movImmToRegOrMem);         return;
        case ADD_IMM_TO_ACC:                                                                 break;
    }

    sb.append("Unknown opcode");
}

Inst8086 decodeInst(core::arr<u8>& bytes, i32& idx){
    auto o   = [](u8 b) -> Opcode { return opcodeDecode(b); };
    auto w   = [](u8 b) -> bool   { return (b >> 0) & 0b1; };
    auto d   = [](u8 b) -> bool   { return (b >> 1) & 0b1; };
    auto s   = [](u8 b) -> bool   { return (b >> 1) & 0b1; };
    [[maybe_unused]] auto v   = [](u8 b) -> bool   { return (b >> 1) & 0b1; };
    auto mod = [](u8 b) -> Mod    { return Mod((b >> 6) & 0b11); };
    auto reg = [](u8 b) -> u8     { return (b >> 3) & 0b111; };
    auto rm  = [](u8 b) -> u8     { return (b >> 0) & 0b111; };

    auto data = [](auto& bytes, i32& idx, u8 w) -> u16 {
        if (w == 0) {
            u16 dataLow = bytes[idx++];
            return dataLow;
        }
        else {
            u16 dataLow = bytes[idx++];
            u16 dataHi = bytes[idx++];
            u16 res = (dataHi << 8) | dataLow;
            return res;
        }
    };

    auto disp = [](auto& bytes, i32& idx, u8 mod) -> u16 {
        if (mod == MOD_MEMORY_8_BIT_DISPLACEMENT) {
            u16 dispLow = bytes[idx++];
            return dispLow;
        }
        else if (mod == MOD_MEMORY_16_BIT_DISPLACEMENT) {
            u16 dispLow = bytes[idx++];
            u16 dispHi = bytes[idx++];
            u16 res = (dispHi << 8) | dispLow;
            return res;
        }
        return 0;
    };

    auto addr = [](auto& bytes, i32& idx) -> u16 {
        u16 addLow = bytes[idx++];
        u16 addrHi = bytes[idx++];
        u16 res = (addrHi << 8) | addLow;
        return res;
    };

    auto inst_v1 = [&](u8 byte1, auto& bytes, i32& idx) -> Inst_v1 {
        u8 byte2 = bytes[idx++];
        Inst_v1 inst = {};
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

        return inst;
    };

    auto inst_v2 = [&](u8 byte1, auto& bytes, i32& idx, bool ignoreW = false) -> Inst_v2 {
        u8 byte2 = bytes[idx++];
        Inst_v2 inst = {};
        inst.w = w(byte1);
        inst.mod = mod(byte2);
        inst.rm = rm(byte2);
        inst.disp = disp(bytes, idx, inst.mod);
        inst.data = data(bytes, idx, ignoreW ? 0 : inst.w);
        return inst;
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
            auto inst = inst_v1(byte1, bytes, idx);
            inst.type = InstType::MOV;
            res.movRegOrMemToOrFromReg = inst;
            return res;
        }
        case MOV_MEM_TO_ACC: [[fallthrough]];
        case MOV_ACC_TO_MEM:
        {
            MovInst_v4 inst = {};
            inst.w = w(byte1);
            inst.d = d(byte1);
            inst.addr = addr(bytes, idx);
            res.movMemToAcc = inst;
            return res;
        }
        case MOV_IMM_TO_REG_OR_MEM:
        {
            auto inst = inst_v2(byte1, bytes, idx);
            inst.type = InstType::MOV;
            res.addImmToRegOrMem = inst;
            return res;
        }
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:
            Assert(false, "Not implemented");
            return res;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:
            Assert(false, "Not implemented");
            return res;

        case ADD_REG_OR_MEM_WITH_REG_TO_EDIT:
        {
            auto inst = inst_v1(byte1, bytes, idx);
            inst.type = InstType::ADD;
            res.addRegOrMemWithRegToEdit = inst;
            return res;
        }
        case ADD_IMM_TO_REG_OR_MEM:
        {
            auto inst = inst_v2(byte1, bytes, idx, true);
            inst.type = InstType::ADD;
            inst.s = s(byte1);
            res.addImmToRegOrMem = inst;
            return res;
        }
        case ADD_IMM_TO_ACC:
            Assert(false, "Not implemented");
            return res;
    }

    return res;
}
