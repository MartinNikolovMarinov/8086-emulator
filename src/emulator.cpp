#include "emulator.h"
#include "emulator_utils.h"

namespace asm8086 {

namespace {

bool isDirectAddrMode(Mod mod, u8 rm) {
    return mod == Mod::MEMORY_NO_DISPLACEMENT && rm == 0b110;
}

bool isRegToReg(Mod mod) {
    return mod == Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT;
}

bool isEffectiveAddrCalc(Mod mod) {
    return mod != Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT;
}

bool isDisplacementMode(Mod mod) {
    return mod == Mod::MEMORY_8_BIT_DISPLACEMENT ||
           mod == Mod::MEMORY_16_BIT_DISPLACEMENT;
}

bool is8bitDisplacement(Mod mod) {
    return mod == Mod::MEMORY_8_BIT_DISPLACEMENT;
}

bool is16bitDisplacement(Mod mod) {
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

    if (ctx.options & DE_FLAG_IMMEDIATE_AS_SIGNED) {
        if (high != 0) appendSigned(sb, i16((high << 8) | low), explictSign);
        else appendSigned(sb, i8(low), explictSign);
    }
    else if (ctx.options & DE_FLAG_IMMEDIATE_AS_UNSIGNED) {
        if (explictSign) sb.append("+ ");
        appendU16toSb(sb, u16((high << 8) | low));
    }
    else if (ctx.options & DE_FLAG_IMMEDIATE_AS_HEX) {
        char ncptr[5] = {};
        core::int_to_hex(u16((high << 8) | low), ncptr);
        if (explictSign) sb.append("+ ");
        sb.append("0x");
        sb.append(ncptr);
    }
    else {
        // Default ot using signed
        if (high != 0) appendSigned(sb, i16((high << 8) | low), explictSign);
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

EmulationContext createEmulationCtx(core::arr<Instruction>&& instructions, EmulationOptionFlags options) {
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

namespace {

void setRegister(EmulationContext& ctx, Register& dst, u8 low, u8 high) {
    u16 old = dst.value;
    if (high != 0) {
        u16 data = (u16(high) << 8) | u16(low);
        dst.value = data;
    }
    else {
        u8 data = low;
        dst.value = (dst.value & 0xFF00) | data;
    }

    if (ctx.options & EmulationOptionFlags::EMU_FLAG_VERBOSE) {
        static i64 counter = 0;
        fmt::print("({}) {}:{:#0x}->{:#0x}", ++counter, regTypeToCptr(dst.type), old, dst.value);
    }
}

Register& getRegister(EmulationContext& ctx, u8 rm, bool isSegment) {
    Register& ret = !isSegment ?
                        ctx.registers[i32(rm)] :
                        ctx.registers[i32(RegisterType::ES) + i32(rm)];
    return ret;
}

void emulateMov(EmulationContext& ctx, const Instruction& inst) {
    switch (inst.operands) {
        case Operands::Register_Immediate:
        {
            // TODO: Cases where mod is effective address calculation, this is nonsense...
            Register& dst = getRegister(ctx, inst.reg, false);
            setRegister(ctx, dst, inst.data[0], inst.data[1]);
            break;
        }
        case Operands::Register_Register:
        {
            Register& src = getRegister(ctx, inst.rm, false);
            Register& dst = getRegister(ctx, inst.reg, false);
            setRegister(ctx, dst, u8(src.value & 0xFF), u8(src.value >> 8));
            break;
        }
        case Operands::Register16_SegReg:
        {
            Register& src = getRegister(ctx, inst.rm, false);
            Register& dst = getRegister(ctx, inst.reg, true);
            setRegister(ctx, dst, u8(src.value & 0xFF), u8(src.value >> 8));
            break;
        }
        case Operands::SegReg_Register16:
        {
            Register& src = getRegister(ctx, inst.rm, true);
            Register& dst = getRegister(ctx, inst.reg, false);
            setRegister(ctx, dst, u8(src.value & 0xFF), u8(src.value >> 8));
            break;
        }
        default:
            Panic(false, "Instruction not supported yet.");
            break;
    }
}

} // namespace

void emulate(EmulationContext& ctx) {
    for (addr_size i = 0; i < ctx.instructions.len(); i++) {
        auto inst = ctx.instructions[i];
        if (isDisplacementMode(inst.mod)) Panic(false, "Can't handle displacement mode yet.");

        switch (inst.type) {
            case InstType::MOV:
            {
                emulateMov(ctx, inst);
                fmt::print("\n");
                break;
            }
            default:
                Panic(false, "Instruction type not supported yet.");
                break;
        }
    }
}

} // namespace asm8086
