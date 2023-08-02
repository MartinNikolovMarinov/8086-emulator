#include "inst_8086.h"
#include "utils.h"

namespace asm8086 {

namespace {

bool isDirectAddressing(Mod mod, u8 rm) {
    return mod == MOD_MEMORY_NO_DISPLACEMENT && rm == 0b110;
}

} // namespace

Inst decodeAsm8086(core::arr<u8>& bytes, i32& idx) {
    auto o   = [](u8 b) -> Opcode { return opcodeDecode(b); };
    auto w   = [](u8 b) -> bool   { return (b >> 0) & 0b1; };
    auto d   = [](u8 b) -> bool   { return (b >> 1) & 0b1; };
    auto mod = [](u8 b) -> Mod    { return Mod((b >> 6) & 0b11); };
    auto reg = [](u8 b) -> u8     { return (b >> 3) & 0b111; };
    auto rm  = [](u8 b) -> u8     { return (b >> 0) & 0b111; };

    auto parseDisp = [](u8 disp[2], Mod mod, u8 rm, auto& bytes, i32& idx) {
        if (mod == MOD_MEMORY_8_BIT_DISPLACEMENT) {
            disp[0] = bytes[idx++];
        }
        else if (mod == MOD_MEMORY_16_BIT_DISPLACEMENT || isDirectAddressing(mod, rm)) {
            disp[0] = bytes[idx++];
            disp[1] = bytes[idx++];
        }
    };

    auto parseData = [](u8 data[2], u8 w, auto& bytes, i32& idx) {
        if (w) {
            data[0] = bytes[idx++];
            data[1] = bytes[idx++];
        }
        else {
            data[0] = bytes[idx++];
        }
    };

    auto createImmToReg = [&](u8 opcodeByte, auto& bytes, i32& idx) -> ImmToReg {
        ImmToReg inst = {};
        inst.w = (opcodeByte >> 3) & 0b1;
        inst.reg = opcodeByte & 0b111;
        parseData(inst.data, inst.w, bytes, idx);
        return inst;
    };

    auto createRegOrMemToOrFromReg = [&](u8 opcodeByte, auto& bytes, i32& idx) -> RegMemToFromReg {
        u8 addrModByte = bytes[idx++];
        RegMemToFromReg inst = {};
        inst.d = d(opcodeByte);
        inst.w = w(opcodeByte);
        inst.mod = mod(addrModByte);
        inst.reg = reg(addrModByte);
        inst.rm = rm(addrModByte);
        parseDisp(inst.disp, inst.mod, inst.rm, bytes, idx);
        return inst;
    };

    auto createAccToMem = [&](u8 opcodeByte, i32& idx) -> MemAccToAccMem {
        MemAccToAccMem inst = {};
        inst.d = d(opcodeByte);
        inst.w = w(opcodeByte);
        parseData(inst.addr, 1, bytes, idx);
        return inst;
    };

    auto createImmToRegOrMem = [&](u8 opcodeByte, auto& bytes, i32& idx, u8* wOverride = nullptr) -> ImmToRegMem {
        u8 addrModByte = bytes[idx++];
        ImmToRegMem inst = {};
        inst.w = w(opcodeByte);
        inst.mod = mod(addrModByte);
        inst.reg = reg(addrModByte);
        inst.rm = rm(addrModByte);
        parseDisp(inst.disp, inst.mod, inst.rm, bytes, idx);
        parseData(inst.data, (wOverride == nullptr) ? inst.w : *wOverride, bytes, idx);
        return inst;
    };

    auto createImmToAcc = [&](u8 opcodeByte, i32& idx) -> ImmToAcc {
        ImmToAcc inst = {};
        inst.w = w(opcodeByte);
        parseData(inst.data, inst.w, bytes, idx);
        return inst;
    };

    i32 currIdx = idx;
    defer { idx += currIdx - idx; };
    u8 opcodeByte = bytes[currIdx++];
    Inst res = {};
    res.opcode = o(opcodeByte);

    switch (res.opcode) {
        case MOV_IMM_TO_REG:
            res.immToReg = createImmToReg(opcodeByte, bytes, currIdx);
            res.type = InstType::MOV;
            break;
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:
            res.regMemToFromReg = createRegOrMemToOrFromReg(opcodeByte, bytes, currIdx);
            res.type = InstType::MOV;
            break;
        case MOV_MEM_TO_ACC: [[fallthrough]];
        case MOV_ACC_TO_MEM:
        {
            res.memAccToAccMem = createAccToMem(opcodeByte, currIdx);
            res.type = InstType::MOV;
            break;
        }
        case MOV_IMM_TO_REG_OR_MEM:
        {
            res.immToRegMem = createImmToRegOrMem(opcodeByte, bytes, currIdx);
            res.type = InstType::MOV;
            break;
        }
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:
        {
            Assert(false, "Not implemented");
            break;
        }
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:
        {
            Assert(false, "Not implemented");
            break;
        }

        case ADD_REG_OR_MEM_WITH_REG_TO_EDIT:
        {
            res.regMemToFromReg = createRegOrMemToOrFromReg(opcodeByte, bytes, currIdx);
            res.type = InstType::ADD;
            break;
        }
        case IMM_TO_FROM_REG_OR_MEM:
        {
            u8 wOverride = 0;
            res.immToRegMem = createImmToRegOrMem(opcodeByte, bytes, currIdx, &wOverride);

            // The type of instruction is deduced by the reg field in this lovely case.
            switch (res.immToRegMem.reg) {
                case 0b000: res.type = InstType::ADD; break;
                case 0b101: res.type = InstType::SUB; break;
                default: Panic(false, "[BUG] Failed to set instruction type");
            }

            break;
        }
        case ADD_IMM_TO_ACC:
        {
            res.immToAcc = createImmToAcc(opcodeByte, currIdx);
            res.type = InstType::ADD;
            break;
        }

        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT:
        {
            res.regMemToFromReg = createRegOrMemToOrFromReg(opcodeByte, bytes, currIdx);
            res.type = InstType::SUB;
            break;
        }
        case SUB_IMM_FROM_ACC:
        {
            res.immToAcc = createImmToAcc(opcodeByte, currIdx);
            res.type = InstType::SUB;
            break;
        }
    }

    return res;
}

namespace {

const char* instTypeToCptr(InstType t) {
    switch (t) {
        case InstType::MOV: return "mov";
        case InstType::ADD: return "add";
        case InstType::SUB: return "sub";
        case InstType::SENTINEL: break;
    }
    return "unknown";
}
                                        // 000  001   010   011   100   101   110   111
constexpr const char* reg8bitTable[]  = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" }; // w = 0
constexpr const char* reg16bitTable[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" }; // w = 1

                                        //   000       001         010        011     100   101   110   111
constexpr const char* regDispTable[]  = { "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx" }; // w = 0

void appendReg(core::str_builder<>& sb, u8 reg, bool w) {
    if (w) {
        sb.append(reg16bitTable[reg]);
    }
    else {
        sb.append(reg8bitTable[reg]);
    }
}

void appendImmediate(core::str_builder<>& sb, u16 i) {
    char ncptr[8] = {};
    core::int_to_cptr(u32(i), ncptr);
    sb.append(ncptr);
}

void appendImmediateSigned(core::str_builder<>& sb, i16 i, bool explictSign = true) {
    if (i < 0) {
        if (explictSign) sb.append("- ");
        else sb.append("-");
        i = -i;
    }
    else {
        if (explictSign) sb.append("+ ");
    }

    char ncptr[8] = {};
    core::int_to_cptr(i32(i), ncptr);
    sb.append(ncptr);
}

void appendRegDisp(core::str_builder<>& sb, u8 reg, u16 disp) {
    sb.append(regDispTable[reg]);
    if (disp != 0) {
        sb.append(" ");
        appendImmediateSigned(sb, i16(disp), true);
    }
}

void appendEffectiveCalc(core::str_builder<>& sb, u8 regOrRm, u16 disp, bool isDirectAddressing) {
    if (isDirectAddressing) {
        appendImmediate(sb, disp);
    } else {
        appendRegDisp(sb, regOrRm, disp);
    }
}

void encodeInst(core::str_builder<>& sb, const RegMemToFromReg& inst) {
    u8 d = inst.d;
    u8 w = inst.w;
    u8 reg = inst.reg;
    u8 rm = inst.rm;
    Mod mod = inst.mod;
    u8 dispLow = inst.disp[0];
    u8 dispHigh = inst.disp[1];

    if (mod == MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT) {
        if (d) {
            appendReg(sb, reg, w); sb.append(", "); appendReg(sb, rm, w);
        }
        else {
            appendReg(sb, rm, w); sb.append(", "); appendReg(sb, reg, w);
        }
    }
    else {
        bool dispIsWord = mod == MOD_MEMORY_8_BIT_DISPLACEMENT ? false : true;
        u16 disp = u16FromLowAndHi(dispIsWord, dispLow, dispHigh);
        if (d) {
            appendReg(sb, reg, w);
            sb.append(", ");
            sb.append("[");
            appendEffectiveCalc(sb, rm, disp, isDirectAddressing(mod, rm));
            sb.append("]");
        }
        else {
            sb.append("[");
            appendEffectiveCalc(sb, rm, disp, isDirectAddressing(mod, rm));
            sb.append("]");
            sb.append(", ");
            appendReg(sb, reg, w);
        }
    }
}

void encodeInst(core::str_builder<>& sb, const ImmToReg& inst) {
    u8 w = inst.w;
    u8 reg = inst.reg;
    u8 dataLow = inst.data[0];
    u8 dataHigh = inst.data[1];
    u16 data = u16FromLowAndHi((w == 1), dataLow, dataHigh);

    appendReg(sb, reg, w);
    sb.append(", ");
    appendImmediateSigned(sb, data, false);
}

void encodeInst(core::str_builder<>& sb, const ImmToRegMem& inst) {
    u8 w = inst.w;
    auto rm = inst.rm;
    auto mod = inst.mod;
    u8 dispLow = inst.disp[0];
    u8 dispHigh = inst.disp[1];
    u8 dataLow = inst.data[0];
    u8 dataHigh = inst.data[1];
    u16 disp = u16FromLowAndHi((w == 1), dispLow, dispHigh);
    u16 data = u16FromLowAndHi((w == 1), dataLow, dataHigh);

    if (mod == MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT) {
        appendReg(sb, rm, w);
        sb.append(", ");
        appendImmediate(sb, data);
    }
    else {
        sb.append(w ? "word " : "byte ");
        sb.append("[");
        appendEffectiveCalc(sb, rm, disp, false);
        sb.append("]");
        sb.append(", ");
        appendImmediate(sb, data);
    }
}

void encodeInst(core::str_builder<>& sb, const MemAccToAccMem& inst) {
    u8 d = inst.d;
    u8 w = inst.w;
    u8 reg = 0b000;
    u8 addrLow = inst.addr[0];
    u8 addrHigh = inst.addr[1];
    u16 addr = u16FromLowAndHi((w == 1), addrLow, addrHigh);

    if (d) {
        sb.append("[");
        appendImmediate(sb, addr);
        sb.append("]");
        sb.append(", ");
        appendReg(sb, reg, w);
    }
    else {
        appendReg(sb, reg, w);
        sb.append(", ");
        sb.append("[");
        appendImmediate(sb, addr);
        sb.append("]");
    }
}

void encodeInst(core::str_builder<>& sb, const ImmToAcc& inst) {
    u8 w = inst.w;
    u8 reg = 0b000;
    u8 dataLow = inst.data[0];
    u8 dataHigh = inst.data[1];
    u16 data = u16FromLowAndHi((w == 1), dataLow, dataHigh);

    appendReg(sb, reg, w);
    sb.append(", ");
    appendImmediateSigned(sb, data, false);
}

}

void encodeAsm8086(core::str_builder<>& sb, const Inst& inst) {
    sb.append(instTypeToCptr(inst.type));
    sb.append(" ");

    switch (inst.opcode) {
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:   encodeInst(sb, inst.regMemToFromReg); return;
        case MOV_IMM_TO_REG:                  encodeInst(sb, inst.immToReg);        return;
        case MOV_IMM_TO_REG_OR_MEM:           encodeInst(sb, inst.immToRegMem);     return;
        case MOV_ACC_TO_MEM:                                                        [[fallthrough]];
        case MOV_MEM_TO_ACC:                  encodeInst(sb, inst.memAccToAccMem);  return;

        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG: break;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY: break;

        case ADD_REG_OR_MEM_WITH_REG_TO_EDIT:  encodeInst(sb, inst.regMemToFromReg); return;
        case IMM_TO_FROM_REG_OR_MEM:           encodeInst(sb, inst.immToRegMem);     return;
        case ADD_IMM_TO_ACC:                   encodeInst(sb, inst.immToAcc);        return;

        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT:  encodeInst(sb, inst.regMemToFromReg); return;
        case SUB_IMM_FROM_ACC:                 encodeInst(sb, inst.immToAcc);        return;
    }

    sb.append("encoding failed");
}

} // namespace asm8086
