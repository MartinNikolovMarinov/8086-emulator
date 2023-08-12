#include "emulator.h"
#include "emulator_utils.h"

namespace asm8086 {

namespace {

bool isDirectAddrMode(Mod mod, u8 rm) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::MEMORY_NO_DISPLACEMENT && rm == 0b110;
}

bool isRegToReg(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT;
}

bool isEffectiveAddrCalc(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod != Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT;
}

bool isDisplacementMode(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::MEMORY_8_BIT_DISPLACEMENT ||
           mod == Mod::MEMORY_16_BIT_DISPLACEMENT;
}

bool is8bitDisplacement(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::MEMORY_8_BIT_DISPLACEMENT;
}

bool is16bitDisplacement(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::MEMORY_16_BIT_DISPLACEMENT;
}

Instruction decodeInstruction(core::arr<u8>& bytes, DecodingContext& ctx) {
    auto decodeFromDisplacements = [&](auto& bytes, addr_off idx, const FieldDisplacements& fd, Instruction& inst) {
        i8 ibc = 0;
        if (fd.d.byteIdx >= 0) {
            ibc = core::max(ibc, fd.d.byteIdx);
            inst.d = (bytes[idx + ibc] & fd.d.mask) >> fd.d.offset;
        }
        if (fd.s.byteIdx >= 0) {
            ibc = core::max(ibc, fd.s.byteIdx);
            inst.s = (bytes[idx + ibc] & fd.s.mask) >> fd.s.offset;
        }
        if (fd.w.offset >= 0) {
            ibc = core::max(ibc, fd.w.byteIdx);
            inst.w = (bytes[idx + ibc] & fd.w.mask) >> fd.w.offset;
        }
        if (fd.mod.byteIdx >= 0) {
            ibc = core::max(ibc, fd.mod.byteIdx);
            inst.mod = Mod((bytes[idx + ibc] & fd.mod.mask) >> fd.mod.offset);
        }
        else {
            inst.mod = Mod::NONE_SENTINEL;
        }
        if (fd.reg.byteIdx >= 0) {
            ibc = core::max(ibc, fd.reg.byteIdx);
            inst.reg = (bytes[idx + ibc] & fd.reg.mask) >> fd.reg.offset;
        }
        if (fd.rm.byteIdx >= 0) {
            ibc = core::max(ibc, fd.rm.byteIdx);
            inst.rm = (bytes[idx + ibc] & fd.rm.mask) >> fd.rm.offset;
        }

        // Displacement byte(s) decoding.
        if (fd.disp1.byteIdx > 0 && is8bitDisplacement(inst.mod)) {
            inst.disp[0] = bytes[idx + ibc + 1];
            ibc++;
        }
        else if (fd.disp1.byteIdx > 0 && (is16bitDisplacement(inst.mod) || isDirectAddrMode(inst.mod, inst.rm))) {
            inst.disp[0] = bytes[idx + ibc + 1];
            inst.disp[1] = bytes[idx + ibc + 2];
            ibc += 2;
        }

        bool dataIsWord = false;
        if (fd.fixedWord < 0) {
            // Size is determined by the w or s bits.
            dataIsWord = inst.s ? false : (inst.w == 1);
        } else {
            // This is a bit hacky, I should have just created a way to default set some fields/bits.
            dataIsWord = fd.fixedWord == 1;
        }

        // Data byte(s) decoding.
        if (fd.data1.byteIdx > 0 && !dataIsWord) {
            inst.data[0] = bytes[idx + ibc + 1];
            ibc++;
        }
        else if (fd.data1.byteIdx > 0 && dataIsWord) {
            inst.data[0] = bytes[idx + ibc + 1];
            inst.data[1] = bytes[idx + ibc + 2];
            ibc += 2;
        }

        inst.byteCount += i8(ibc + 1);
    };

    auto storeShortJmpLabel = [](core::arr<JmpLabel>& jmpLabels, const Instruction &inst, addr_off idx) {
        addr_off diff = 0;
        i8 shortJmpDiff = i8(inst.data[0]);
        safeCastSignedInt(shortJmpDiff, diff);
        addr_off byteOff = addr_off(idx) + addr_off(inst.byteCount) + addr_off(diff);
        JmpLabel jmpLabel = { byteOff, addr_off(jmpLabels.len()) };
        core::appendUnique(jmpLabels, jmpLabel, [&](auto& x) -> bool { return x.byteOffset == byteOff; });
    };

    auto& jmpLabels = ctx.jmpLabels;

    addr_off idx = addr_off(ctx.idx);
    Opcode opcode = opcodeDecode(bytes[idx]);
    auto fd = getFieldDisplacements(opcode);

    Instruction inst = {};
    inst.opcode = opcode;
    decodeFromDisplacements(bytes, idx, fd, inst);

    switch (inst.opcode) {
        case MOV_IMM_TO_REG:
            inst.operands = Operands::Register_Immediate;
            inst.rm = inst.reg;
            inst.type = InstType::MOV;
            break;
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:
            if (isRegToReg(inst.mod)) inst.operands = Operands::Register_Register;
            else inst.operands = inst.d ? Operands::Memory_Register : Operands::Register_Memory;
            inst.type = InstType::MOV;
            break;
        case MOV_MEM_TO_ACC:
            inst.operands = Operands::Memory_Accumulator;
            inst.type = InstType::MOV;
            break;
        case MOV_ACC_TO_MEM:
            inst.operands = Operands::Accumulator_Memory;
            inst.type = InstType::MOV;
            break;
        case MOV_IMM_TO_REG_OR_MEM:
            inst.operands = isEffectiveAddrCalc(inst.mod) ? Operands::Memory_Immediate : Operands::Register_Immediate;
            inst.type = InstType::MOV;
            break;
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:
            inst.operands = isRegToReg(inst.mod) ? Operands::Register16_SegReg : Operands::Memory_SegReg;
            inst.type = InstType::MOV;
            break;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:
            inst.operands = isRegToReg(inst.mod)  ? Operands::SegReg_Register16 : Operands::SegReg_Memory16;
            inst.type = InstType::MOV;
            break;

        case IMM_TO_FROM_REG_OR_MEM:
        {
            // The type of instruction is deduced by the reg field in this lovely case.
            switch (inst.reg) {
                case 0b000:
                    inst.operands = isEffectiveAddrCalc(inst.mod) ? Operands::Memory_Immediate : Operands::Register_Immediate;
                    inst.type = InstType::ADD;
                    break;
                case 0b101:
                    inst.operands = isEffectiveAddrCalc(inst.mod) ? Operands::Memory_Immediate : Operands::Register_Immediate;
                    inst.type = InstType::SUB;
                    break;
                case 0b111:
                    inst.operands = isEffectiveAddrCalc(inst.mod) ? Operands::Memory_Immediate : Operands::Register_Immediate;
                    inst.type = InstType::CMP;
                    break;
                default:
                    Panic(false, "[BUG] Failed to set instruction type");
            }

            break;
        }

        case ADD_REG_OR_MEM_WITH_REG_TO_EDIT:
            if (isRegToReg(inst.mod)) inst.operands = Operands::Register_Register;
            else inst.operands = inst.d ? Operands::Memory_Register : Operands::Register_Memory;
            inst.type = InstType::ADD;
            break;
        case ADD_IMM_TO_ACC:
            inst.operands = Operands::Accumulator_Immediate;
            inst.type = InstType::ADD;
            break;

        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT:
            if (isRegToReg(inst.mod)) inst.operands = Operands::Register_Register;
            else inst.operands = inst.d ? Operands::Memory_Register : Operands::Register_Memory;
            inst.type = InstType::SUB;
            break;
        case SUB_IMM_FROM_ACC:
            inst.operands = Operands::Accumulator_Immediate;
            inst.type = InstType::SUB;
            break;

        case CMP_REG_OR_MEM_WITH_REG:
            if (isRegToReg(inst.mod)) inst.operands = Operands::Register_Register;
            else inst.operands = inst.d ? Operands::Memory_Register : Operands::Register_Memory;
            inst.type = InstType::CMP;
            break;
        case CMP_IMM_WITH_ACC:
            inst.operands = Operands::Accumulator_Immediate;
            inst.type = InstType::CMP;
            break;

        case JE_JZ_ON_EQ_ZERO:
            inst.type = InstType::JE;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JL_JNGE_ON_LESS_NOT_GE_OR_EQ:
            inst.type = InstType::JL;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JLE_JNG_ON_LESS_OR_EQ_NOT_GE:
            inst.type = InstType::JLE;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JB_JNAE_ON_BELOW_NOT_ABOVE_OR_EQ:
            inst.type = InstType::JB;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JBE_JNA_ON_BELOW_OR_EQ_NOT_ABOVE:
            inst.type = InstType::JBE;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JP_JPE_ON_PARITY_EVEN:
            inst.type = InstType::JP;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JO_ON_OVERFLOW:
            inst.type = InstType::JO;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JS_ON_SIGN:
            inst.type = InstType::JS;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JNE_JNZ_ON_NOT_EQ_NOR_ZERO:
            inst.type = InstType::JNE;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JNL_JGE_ON_NOT_LESS_GE_OR_EQ:
            inst.type = InstType::JNL;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JNLE_JG_ON_NOT_LESS_OR_EQ_GE:
            inst.type = InstType::JNLE;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JNB_JAE_ON_NOT_BELOW_ABOVE_OR_EQ:
            inst.type = InstType::JNB;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JNBE_JA_ON_NOT_BELOW_OR_EQ_ABOVE:
            inst.type = InstType::JNBE;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JNP_JPO_ON_NOT_PAR_PAR_ODD:
            inst.type = InstType::JNP;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JNO_ON_NOT_OVERFLOW:
            inst.type = InstType::JNO;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JNS_ON_NOT_SIGN:
            inst.type = InstType::JNS;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case LOOP_CX_TIMES:
            inst.type = InstType::LOOP;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case LOOPZ_LOOPE_WHILE_ZERO_EQ:
            inst.type = InstType::LOOPE;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case LOOPNZ_LOOPNE_WHILE_NOT_ZERO_EQ:
            inst.type = InstType::LOOPNE;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
        case JCXZ_ON_CX_ZERO:
            inst.type = InstType::JCXZ;
            inst.operands = Operands::ShortLabel;
            storeShortJmpLabel(jmpLabels, inst, idx);
            break;
    }

    return inst;
}

} // namespace

void decodeAsm8086(core::arr<u8>& bytes, DecodingContext& ctx) {
    while (ctx.idx < bytes.len()) {
        auto inst = decodeInstruction(bytes, ctx);
        ctx.idx += inst.byteCount;
        ctx.instructions.append(inst);
    }
}

const char* instTypeToCptr(InstType t) {
    switch (t) {
        case InstType::MOV:      return "mov";
        case InstType::ADD:      return "add";
        case InstType::SUB:      return "sub";
        case InstType::CMP:      return "cmp";
        case InstType::JE:       return "je";
        case InstType::JZ:       return "jz";
        case InstType::JL:       return "jl";
        case InstType::JNGE:     return "jnge";
        case InstType::JLE:      return "jle";
        case InstType::JNG:      return "jng";
        case InstType::JB:       return "jb";
        case InstType::JNAE:     return "jnae";
        case InstType::JBE:      return "jbe";
        case InstType::JNA:      return "jna";
        case InstType::JP:       return "jp";
        case InstType::JPE:      return "jpe";
        case InstType::JO:       return "jo";
        case InstType::JS:       return "js";
        case InstType::JNE:      return "jne";
        case InstType::JNZ:      return "jnz";
        case InstType::JNL:      return "jnl";
        case InstType::JGE:      return "jge";
        case InstType::JNLE:     return "jnle";
        case InstType::JG:       return "jg";
        case InstType::JNB:      return "jnb";
        case InstType::JAE:      return "jae";
        case InstType::JNBE:     return "jnbe";
        case InstType::JA:       return "ja";
        case InstType::JNP:      return "jnp";
        case InstType::JPO:      return "jpo";
        case InstType::JNO:      return "jno";
        case InstType::JNS:      return "jns";
        case InstType::LOOP:     return "loop";
        case InstType::LOOPE:    return "loope";
        case InstType::LOOPZ:    return "loopz";
        case InstType::LOOPNE:   return "loopne";
        case InstType::LOOPNZ:   return "loopnz";
        case InstType::JCXZ:     return "jcxz";
        case InstType::UNKNOWN:  return "unknown";
        case InstType::SENTINEL: break;
    }
    return "invalid";
}

namespace {

void appendU16toSb(core::str_builder<>& sb, u16 i) {
    char ncptr[8] = {};
    core::int_to_cptr(u32(i), ncptr);
    sb.append(ncptr);
}
GUARD_FN_TYPE_DEDUCTION(appendU16toSb);

void appendImmFromLowAndHigh(core::str_builder<>& sb, const DecodingContext& ctx, bool explictSign, u8 low, u8 high) {
    auto appendSigned = [](core::str_builder<>& sb, auto i, bool explictSign) {
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

    };

    if (ctx.options & DEC_OP_IMMEDIATE_AS_SIGNED) {
        if (high != 0) appendSigned(sb, i16(combineWord(low, high)), explictSign);
        else appendSigned(sb, i8(low), explictSign);
    }
    else if (ctx.options & DEC_OP_IMMEDIATE_AS_UNSIGNED) {
        if (explictSign) sb.append("+ ");
        appendU16toSb(sb, combineWord(low, high));
    }
    else if (ctx.options & DEC_OP_IMMEDIATE_AS_HEX) {
        char ncptr[5] = {};
        core::int_to_hex(combineWord(low, high), ncptr);
        if (explictSign) sb.append("+ ");
        sb.append("0x");
        sb.append(ncptr);
    }
    else {
        // Default ot using signed
        if (high != 0) appendSigned(sb, i16(combineWord(low, high)), explictSign);
        else appendSigned(sb, i8(low), explictSign);
    }
}
                                        // 000  001   010   011   100   101   110   111
constexpr const char* reg8bitTable[]  = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" }; // w = 0
constexpr const char* reg16bitTable[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" }; // w = 1

                                        //   000       001         010        011     100   101   110   111
constexpr const char* regDispTable[]  = { "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx" }; // w = 0

                                        // 000  001   010   011
constexpr const char* segRegTable[]   = { "es", "cs", "ss", "ds" };

void appendReg(core::str_builder<>& sb, u8 reg, bool isWord, bool isSegment = false) {
    if (isSegment) {
        sb.append(segRegTable[reg]);
    }
    else {
        sb.append(isWord ? reg16bitTable[reg] : reg8bitTable[reg]);
    }
}

void appendRegDisp(core::str_builder<>& sb, const DecodingContext& ctx, u8 reg, u8 dispLow, u8 dispHigh) {
    sb.append(regDispTable[reg]);
    if (dispLow != 0 || dispHigh != 0) {
        sb.append(" ");
        appendImmFromLowAndHigh(sb, ctx, true, dispLow, dispHigh);
    }
}

void appendMemory(core::str_builder<>& sb, const DecodingContext& ctx,
                 u8 rm, u8 dispLow, u8 dispHigh, bool isWord, bool isCalc, bool isDirect) {
    if (isDirect) {
        sb.append("[");
        appendImmFromLowAndHigh(sb, ctx, false, dispLow, dispHigh);
        sb.append("]");
    }
    else if (isCalc) {
        sb.append("[");
        appendRegDisp(sb, ctx, rm, dispLow, dispHigh);
        sb.append("]");
    }
    else {
        appendReg(sb, rm, isWord, false);
    }
}

void encodeInstruction(core::str_builder<>& sb,
                       const DecodingContext& ctx,
                       const Instruction& inst,
                       addr_size byteIdx) {
    u8 d = inst.d;
    u8 w = inst.w;
    u8 reg = inst.reg;
    u8 rm = inst.rm;
    Mod mod = inst.mod;
    u8 dispLow = inst.disp[0];
    u8 dispHigh = inst.disp[1];
    u8 dataLow = inst.data[0];
    u8 dataHigh = inst.data[1];
    auto operands = inst.operands;
    bool isCalc = isEffectiveAddrCalc(mod);
    bool isDirect = isDirectAddrMode(mod, rm);
    bool dispIsWord = isDirect ? true : is16bitDisplacement(mod);
    auto& jmpLabels = ctx.jmpLabels;

    auto appendToRegFromImm = [&](u8 dstReg, bool dstIsWord) {
        appendReg(sb, dstReg, dstIsWord, false);
        sb.append(", ");
        appendImmFromLowAndHigh(sb, ctx, false, dataLow, dataHigh);
    };
    auto appendToMemFromImm = [&](u8 dstMem, bool immIsWord) {
        sb.append(immIsWord ? "word " : "byte ");
        appendMemory(sb, ctx, dstMem, dispLow, dispHigh, dispIsWord, isCalc, isDirect);
        sb.append(", ");
        appendImmFromLowAndHigh(sb, ctx, false, dataLow, dataHigh);
    };
    auto appendToRegFromReg = [&](u8 dstReg, bool dstIsSegment,
                                u8 srcReg, bool srcIsSegment, bool areWordRegs) {
        appendReg(sb, dstReg, areWordRegs, dstIsSegment);
        sb.append(", ");
        appendReg(sb, srcReg, areWordRegs, srcIsSegment);
    };
    auto appendToRegFromMem = [&](u8 dstMem, u8 srcReg, bool srcIsSegment, bool srcIsWord) {
        appendMemory(sb, ctx, dstMem, dispLow, dispHigh, dispIsWord, isCalc, isDirect);
        sb.append(", ");
        appendReg(sb, srcReg, srcIsWord, srcIsSegment);
    };
    auto appendToMemFromReg = [&](u8 dstReg, bool dstIsSegment, bool dstIsWord, u8 srcMem) {
        appendReg(sb, dstReg, dstIsWord, dstIsSegment);
        sb.append(", ");
        appendMemory(sb, ctx, srcMem, dispLow, dispHigh, dispIsWord, isCalc, isDirect);
    };
    auto appendMemAcc = [&](bool accRegIsWord, u8 d) {
        if (d) {
            sb.append("[");
            appendImmFromLowAndHigh(sb, ctx, false, dataLow, dataHigh);
            sb.append("]");
            sb.append(", ");
            appendReg(sb, 0b000, accRegIsWord);
        }
        else {
            appendReg(sb, 0b000, accRegIsWord);
            sb.append(", ");
            sb.append("[");
            appendImmFromLowAndHigh(sb, ctx, false, dataLow, dataHigh);
            sb.append("]");
        }
    };
    auto appendToAccFromImm = [&](bool accRegIsWord) {
        appendReg(sb, 0b000, accRegIsWord, false);
        sb.append(", ");
        appendImmFromLowAndHigh(sb, ctx, false, dataLow, dataHigh);
    };
    auto appendShortLabel = [&]() {
        i8 shotJmpOffset = i8(dataLow);
        addr_off byteOff = addr_off(byteIdx) + shotJmpOffset;
        i64 jmpidx = core::find(jmpLabels, [&](auto& el, addr_off) -> bool { return el.byteOffset == byteOff; });
        if (jmpidx == -1) {
            sb.append("(failed to decode label)");
        }
        else {
            sb.append("label_");
            // TODO: I should makeup my mind on how long a jump is allowed. Is there a point to use 64 bit numbers, if yes,
            //       then there should be a function to append them.
            appendU16toSb(sb, u16(jmpLabels[jmpidx].labelIdx));
        }
    };

    // Encode instruction name:
    sb.append(instTypeToCptr(inst.type));
    sb.append(" ");

    switch (operands) {
        case Operands::Accumulator_Memory:    appendMemAcc((w == 1), d); break;
        case Operands::Accumulator_Immediate: appendToAccFromImm((w == 1)); break;

        case Operands::Memory_Accumulator:    appendMemAcc((w == 1), d); break;
        case Operands::Memory_Immediate:      appendToMemFromImm(rm, (w == 1));  break;
        case Operands::Memory_Register:       appendToMemFromReg(reg, false, (w == 1), rm); break;

        case Operands::Register_Register:     appendToRegFromReg(rm, false, reg, false, (w == 1)); break;
        case Operands::Register_Memory:       appendToRegFromMem(rm, reg, false, (w == 1)); break;
        case Operands::Register_Immediate:    appendToRegFromImm(rm, (w == 1)); break;

        case Operands::SegReg_Register16:     appendToRegFromReg(rm, false, reg, true, true); break;
        case Operands::Register16_SegReg:     appendToRegFromReg(reg, true, rm, false, true); break;

        case Operands::SegReg_Memory16:       appendToRegFromMem(rm, reg, true, true); break;
        case Operands::Memory_SegReg:         appendToMemFromReg(reg, true, true, rm); break;

        case Operands::ShortLabel:            appendShortLabel(); break;

        case Operands::SENTINEL:              sb.append("(encoding failed)"); break;
        case Operands::None:                  sb.append("(encoding failed)"); break;
    }
}

}

void encodeAsm8086(core::str_builder<>& asmOut, const DecodingContext& ctx) {
    addr_size byteIdx = 0;
    for (addr_size i = 0; i <= ctx.instructions.len(); i++) {
        // Insert the label before the instruction.
        {
            i64 jmpidx = core::find(ctx.jmpLabels, [&](auto& el, addr_off) -> bool {
                return el.byteOffset == addr_off(byteIdx);
            });
            if (jmpidx != -1) {
                asmOut.append("label_");
                appendU16toSb(asmOut, u64(ctx.jmpLabels[jmpidx].labelIdx));
                asmOut.append(":");
                if (i != ctx.instructions.len()) {
                    // Don't want any empty lines at the end of the file. Ever.
                    asmOut.append("\n");
                }
            }
        }

        if (i == ctx.instructions.len()) {
            // This does one iteration past the end of the instruction array to print labels at the end of the file.
            break;
        }
        auto inst = ctx.instructions[i];
        byteIdx += inst.byteCount; // Intentionally increase the size before encoding.
        encodeInstruction(asmOut, ctx, inst, byteIdx);
        asmOut.append("\n");
    }
}

EmulationContext createEmulationCtx(core::arr<Instruction>&& instructions, EmulationOpts options) {
    EmulationContext ctx;
    ctx.instructions = core::move(instructions);
    ctx.options = options;
    core::memset(ctx.memory, 0, EmulationContext::MEMORY_SIZE);
    for (addr_size i = 0; i < addr_size(RegisterType::SENTINEL); i++) {
        auto& reg = ctx.registers[i];
        reg.type = RegisterType(i);
        reg.value = 0;
    }
    return ctx;
}

const char* modeToCptr(Mod mod) {
    switch (mod) {
        case Mod::MEMORY_NO_DISPLACEMENT:               return "memory_no_displacement";
        case Mod::MEMORY_8_BIT_DISPLACEMENT:            return "memory_8_bit_displacement";
        case Mod::MEMORY_16_BIT_DISPLACEMENT:           return "memory_16_bit_displacement";
        case Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT: return "register_to_register_no_displacement";
        case Mod::NONE_SENTINEL:                        return "none";
        default:                                        return "invalid mode";
    }
}

const char* regTypeToCptr(const RegisterType& rtype) {
    switch (rtype) {
        case RegisterType::AX:    return "ax";
        case RegisterType::CX:    return "cx";
        case RegisterType::DX:    return "dx";
        case RegisterType::BX:    return "bx";
        case RegisterType::SP:    return "sp";
        case RegisterType::BP:    return "bp";
        case RegisterType::SI:    return "si";
        case RegisterType::DI:    return "di";
        case RegisterType::ES:    return "es";
        case RegisterType::CS:    return "cs";
        case RegisterType::SS:    return "ss";
        case RegisterType::DS:    return "ds";
        case RegisterType::IP:    return "ip";
        case RegisterType::FLAGS: return "flags";
        default:                  return "invalid register";
    }
}

const char* operandsToCptr(Operands o) {
    switch (o) {
        case Operands::Memory_Accumulator:    return "memory_accumulator";
        case Operands::Memory_Register:       return "memory_register";
        case Operands::Memory_Immediate:      return "memory_immediate";

        case Operands::Register_Register:     return "register_register";
        case Operands::Register_Memory:       return "register_memory";
        case Operands::Register_Immediate:    return "register_immediate";

        case Operands::Accumulator_Memory:    return "accumulator_memory";
        case Operands::Accumulator_Immediate: return "accumulator_immediate";

        case Operands::ShortLabel:            return "shortlabel";

        case Operands::SegReg_Register16:     return "segreg_register16";
        case Operands::SegReg_Memory16:       return "segreg_memory16";
        case Operands::Register16_SegReg:     return "register16_segreg";
        case Operands::Memory_SegReg:         return "memory_segreg";

        case Operands::None:                  return "none";
        default:                              return "invalid operands";
    }
}

char* instructionToInfoCptr(const Instruction& inst, char* out) {
    const char* opcodeCptr = opcodeToCptr(inst.opcode);
    const char* instType = instTypeToCptr(inst.type);
    char instByteSize[8] = {};
    core::int_to_cptr(u32(inst.byteCount), instByteSize);
    const char* operandsCptr = operandsToCptr(inst.operands);
    const char* modCptr = modeToCptr(inst.mod);

    out = fmt::format_to(out, "Instruction ({}):\n\tType=\"{}\",\n\tByteSize={},\n\tOperands={},\n\tD={}, S={}, W={},\n\t"
                               "Mod=\"{}\"\n\tReg=0b{:03b}, RM=0b{:03b}\n",
                               opcodeCptr, instType, instByteSize, operandsCptr, inst.d, inst.s, inst.w,
                               modCptr, inst.reg, inst.rm);

    if (inst.disp[0] != 0 || inst.disp[1] != 0) {
        out = fmt::format_to(out, "\tDisp={{high:{:#06x},low:{:#06x}}}\n", inst.disp[1], inst.disp[0]);
    }
    if (inst.data[0] != 0 || inst.data[1] != 0) {
        out = fmt::format_to(out, "\tData={{high:{:#06x},low:{:#06x}}}\n", inst.data[1], inst.data[0]);
    }

    return out;
}

char* flagsToCptr(Flags f, char* buffer) {
    buffer[0] = '-';
    if (f & CPU_FLAG_CARRY_FLAG)     *buffer++ = 'C';
    if (f & CPU_FLAG_PARITY_FLAG)    *buffer++ = 'P';
    if (f & CPU_FLAG_AUX_CARRY_FLAG) *buffer++ = 'A';
    if (f & CPU_FLAG_ZERO_FLAG)      *buffer++ = 'Z';
    if (f & CPU_FLAG_SIGN_FLAG)      *buffer++ = 'S';
    if (f & CPU_FLAG_OVERFLOW_FLAG)  *buffer++ = 'O';
    return buffer;
}

namespace {

constexpr inline void setFlag(Register& flags, Flags flag, bool value) {
    flags.value = value ? (flags.value | flag) : (flags.value & ~flag);
}

constexpr inline bool isFlagSet(Register& flags, Flags flag) {
    return (flags.value & flag) != Flags::CPU_FLAG_NONE;
}

Register* getRegister(EmulationContext& ctx, u8 reg, bool isWord, bool isSegment) {
    if (isSegment) {
        Register& ret = ctx.registers[i32(RegisterType::ES) + i32(reg)];
        return &ret;
    }

    if (isWord) {
        Register& ret = ctx.registers[i32(RegisterType::AX) + i32(reg)];
        return &ret;
    }

    // 8-bit register

    switch (reg) {
        // Lower registers:
        case 0b000: return &ctx.registers[i32(RegisterType::AX)];
        case 0b001: return &ctx.registers[i32(RegisterType::CX)];
        case 0b010: return &ctx.registers[i32(RegisterType::DX)];
        case 0b011: return &ctx.registers[i32(RegisterType::BX)];

        // Higher registers:
        case 0b100: return &ctx.registers[i32(RegisterType::AX)];
        case 0b101: return &ctx.registers[i32(RegisterType::CX)];
        case 0b110: return &ctx.registers[i32(RegisterType::DX)];
        case 0b111: return &ctx.registers[i32(RegisterType::BX)];
    }

    Panic(false, "[BUG] failed to get register ");
    return nullptr;
}

Register& getFlagsRegister(EmulationContext& ctx) {
    return ctx.registers[i32(RegisterType::FLAGS)];
}

void emulateMov(Register& dst, bool dstIsLow, bool isWord, u8 srcLow, u8 srcHigh, bool srcIsLow) {
    if (isWord) {
        dst.value = combineWord(srcLow, srcHigh);
        return;
    }

    u8 srcVal = srcIsLow ? srcLow : srcHigh;
    if (dstIsLow) {
        dst.value = combineWord(clearHighPart(dst.value), srcVal);
    }
    else {
        dst.value = combineWord(srcVal, clearLowPart(dst.value));
    }
}

constexpr inline void setOperationFlags(Register& flags, u16 original, u16 next,
                                        bool isWord, bool zeroFlag, bool signFlag, bool carryFlag, bool overflowFlag,
                                        u8 srcLow, u8 srcHigh, bool srcIsLow) {
    setFlag(flags, CPU_FLAG_ZERO_FLAG, zeroFlag);
    setFlag(flags, CPU_FLAG_SIGN_FLAG, signFlag);
    setFlag(flags, CPU_FLAG_CARRY_FLAG, carryFlag);
    setFlag(flags, CPU_FLAG_OVERFLOW_FLAG, overflowFlag);

    // Parity flag

    i32 setBitsCount = __builtin_popcount(lowPart(next));
    setFlag(flags, CPU_FLAG_PARITY_FLAG, (setBitsCount & 0x1) == 0);

    // Auxiliary carry flag

    bool auxCarryFlag = false;
    if (isWord) {
        auxCarryFlag = ((original ^ ((u16(srcHigh) << 8) | u16(srcLow)) ^ next) & 0x10) != 0;
    }
    else {
        u8 originalLow = lowPart(original);
        u8 operand = srcIsLow ? srcLow : srcHigh;
        auxCarryFlag = ((originalLow ^ operand ^ lowPart(next)) & 0x10) != 0;
    }
    setFlag(flags, CPU_FLAG_AUX_CARRY_FLAG, auxCarryFlag);
}

void emulateAdd(Register& dst, Register& flags, bool dstIsLow, bool isWord, u8 srcLow, u8 srcHigh, bool srcIsLow) {
    bool signFlag, zeroFlag, carryFlag, overflowFlag;
    u16 original = dst.value;
    u16 next;

    // FIXME: My assumption for flag setting is completely wrong. The only time adding word numbers sets any of the
    // flags is when the destination register is 8bit register. It is not based on the size of data in register to
    // immediate. When immediate is being store to a register I can assume that I the whole register is used and only
    // set flags on the whole register. Only the cases where specifically al, bl, cl, dl, ah, bh, ch, dh are used I need
    // to set flags based on the 8bit value. That is a bit crazy, but at least I know I am on the right track now.

    if (isWord) {
        u16 src = combineWord(srcLow, srcHigh);
        next = dst.value + src;
        signFlag = i16(next) < 0;
        zeroFlag = next == 0;
        carryFlag = next < original;
        overflowFlag = isSignedBitSet(src) == isSignedBitSet(original) &&
                       isSignedBitSet(src) != isSignedBitSet(next);
    }
    else {
        u8 srcVal = srcIsLow ? srcLow : srcHigh;
        if (dstIsLow) {
            u8 dstLow = lowPart(dst.value);
            dstLow += srcVal;
            next = clearLowPart(dst.value) | u16(dstLow);
            signFlag = i8(dstLow) < 0;
            zeroFlag = dstLow == 0;
            carryFlag = dstLow < lowPart(original);
            overflowFlag = isSignedBitSet(srcVal) == isSignedBitSet(lowPart(original)) &&
                           isSignedBitSet(srcVal) != isSignedBitSet(dstLow);
        }
        else {
            u8 dstHigh = highPart(dst.value);
            dstHigh += srcVal;
            next = combineWord(clearHighPart(dst.value), dstHigh);
            signFlag = i8(dstHigh) < 0;
            zeroFlag = dstHigh == 0;
            carryFlag = dstHigh < highPart(original);
            overflowFlag = isSignedBitSet(srcVal) == isSignedBitSet(highPart(original)) &&
                           isSignedBitSet(srcVal) != isSignedBitSet(dstHigh);
        }
    }

    dst.value = next;

    setOperationFlags(flags, original, next, isWord, zeroFlag, signFlag, carryFlag, overflowFlag, srcLow, srcHigh, srcIsLow);
}

void emulateSub(Register& dst, Register& flags, bool dstIsLow, bool isWord, u8 srcLow, u8 srcHigh, bool srcIsLow) {
    bool signFlag, zeroFlag, carryFlag, overflowFlag;
    u16 original = dst.value;
    u16 next;

    if (isWord) {
        u16 src = combineWord(srcLow, srcHigh);
        next = dst.value - src;
        signFlag = i16(next) < 0;
        zeroFlag = next == 0;
        carryFlag = original < next;
        overflowFlag = (isSignedBitSet(original) != isSignedBitSet(src)) &&
                       (isSignedBitSet(original) != isSignedBitSet(next));
    }
    else {
        u8 srcVal = srcIsLow ? srcLow : srcHigh;
        if (dstIsLow) {
            u8 dstLow = lowPart(dst.value);
            dstLow -= srcVal;
            next = clearLowPart(dst.value) | u16(dstLow);
            signFlag = i8(dstLow) < 0;
            zeroFlag = dstLow == 0;
            carryFlag = lowPart(original) < dstLow;
            overflowFlag = (isSignedBitSet(lowPart(original)) != isSignedBitSet(srcVal)) &&
                           (isSignedBitSet(lowPart(original)) != isSignedBitSet(dstLow));
        }
        else {
            u8 dstHigh = highPart(dst.value);
            dstHigh -= srcVal;
            next = combineWord(clearHighPart(dst.value), dstHigh);
            signFlag = i8(dstHigh) < 0;
            zeroFlag = dstHigh == 0;
            carryFlag = highPart(original) < dstHigh;
            overflowFlag = (isSignedBitSet(highPart(original)) != isSignedBitSet(srcVal)) &&
                           (isSignedBitSet(highPart(original)) != isSignedBitSet(dstHigh));
        }
    }

    dst.value = next;

    setOperationFlags(flags, original, next, isWord, zeroFlag, signFlag, carryFlag, overflowFlag, srcLow, srcHigh, srcIsLow);
}

void emulateNext(EmulationContext& ctx, const Instruction& inst) {
    Register* dst = nullptr;
    bool dstIsLow = false;
    u8 srcLow = 0; u8 srcHigh = 0;
    bool srcIsLow = false;
    bool isWord = inst.s ? false : (inst.w == 1);

    switch (inst.operands) {
        case Operands::Register_Immediate:
        {
            dst = getRegister(ctx, inst.rm, isWord, false);
            dstIsLow = isLowRegister(inst.rm);
            srcLow = inst.data[0];
            srcHigh = inst.data[1];
            srcIsLow = true;
            break;
        }
        case Operands::Register_Register:
        {
            dst = getRegister(ctx, inst.rm, isWord, false);
            dstIsLow = isLowRegister(inst.rm);
            Register* src = getRegister(ctx, inst.reg, isWord, false);
            srcLow = lowPart(src->value);
            srcHigh = highPart(src->value);
            srcIsLow = isLowRegister(inst.reg);
            break;
        }
        case Operands::Register16_SegReg:
        {
            dst = getRegister(ctx, inst.reg, true, true);
            Register* src = getRegister(ctx, inst.rm, true, false);
            srcLow = lowPart(src->value);
            srcHigh = highPart(src->value);
            isWord = true;
            break;
        }
        case Operands::SegReg_Register16:
        {
            dst = getRegister(ctx, inst.rm, true, false);
            Register* src = getRegister(ctx, inst.reg, true, true);
            srcLow = lowPart(src->value);
            srcHigh = highPart(src->value);
            isWord = true;
            break;
        }
        case Operands::ShortLabel:
        {
            isWord = false;
            break;
        }
        default:
            Panic(false, "Instruction not supported yet.");
            break;
    }

    u16 old = dst ? dst->value : 0;
    Register& ip = ctx.registers[i32(RegisterType::IP)];
    i16 deltaIp = 0;

    switch (inst.type) {
        case InstType::MOV:
            Assert(dst);
            emulateMov(*dst, dstIsLow, isWord, srcLow, srcHigh, srcIsLow);
            break;
        case InstType::ADD:
            Assert(dst);
            emulateAdd(*dst, getFlagsRegister(ctx), dstIsLow, isWord, srcLow, srcHigh, srcIsLow);
            break;
        case InstType::SUB:
            Assert(dst);
            emulateSub(*dst, getFlagsRegister(ctx), dstIsLow, isWord, srcLow, srcHigh, srcIsLow);
            break;
        case InstType::CMP:
            Assert(dst);
            emulateSub(*dst, getFlagsRegister(ctx), dstIsLow, isWord, srcLow, srcHigh, srcIsLow);
            dst->value = old; // cmp is the same as sub, but doesn't write to dst
            break;
        case InstType::JNZ:
        case InstType::JNE:
        {
            // TODO: I should write a function for this, once I know how to abstract it.
            Register& flags = getFlagsRegister(ctx);
            if (isFlagSet(flags, Flags::CPU_FLAG_ZERO_FLAG) == false) {
                deltaIp += i8(inst.data[0]);
            }
            break;
        }
        default:
            Panic(false, "Instruction not supported yet.");
            break;
    }

    if (ctx.options & EmulationOpts::EMU_OPT_VERBOSE &&
        /* FIXME:
            This dst != nullptr check  is a hack to avoid exploding on instructions that are not operations. I
            should rethink the way printing happens right now. It seems to be a bit more instruction specific than I
            expected.
        */
        dst != nullptr
    ) {
        static i64 tmp_g_counter = 0;
        char flagsBuf[BUFFER_SIZE_FLAGS] = {};
        flagsToCptr(Flags(getFlagsRegister(ctx).value), flagsBuf);
        fmt::print("({}) {}, {}:{:#0x}->{:#0x}, ip: {:#0x}, flags: {}\n",
                    ++tmp_g_counter, instTypeToCptr(inst.type),
                    regTypeToCptr(dst->type), old, dst->value, ip.value, flagsBuf);
    }

    ip.value += deltaIp + inst.byteCount;
}

bool nextInst(const EmulationContext& ctx, Instruction& inst) {
    // FIXME: Linear search on each instruction is the hacky way to make jumps work, for now. Once there is a proper
    //        address table that can map into the instructions this will be trivial to fix.
    const Register& ip = ctx.registers[i32(RegisterType::IP)];
    addr_off byteOff = 0;
    i64 idx = core::find(ctx.instructions, [&](const auto& el, addr_off) -> bool {
        bool ret = ip.value == byteOff;
        byteOff += el.byteCount;
        return ret;
    });
    if (idx == -1) {
        return false;
    }
    inst = ctx.instructions[idx];
    return true;
}

} // namespace

void emulate(EmulationContext& ctx) {
    Instruction inst;
    while (nextInst(ctx, inst)) {
#if 0
        // Print the instruction info:
        char info[BUFFER_SIZE_INST_INFO_OUT] = {};
        instructionToInfoCptr(inst, info);
        fmt::print("{}\n", info);
#endif
        emulateNext(ctx, inst);
    }
}

} // namespace asm8086
