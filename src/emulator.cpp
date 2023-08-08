#include "emulator.h"
#include "emulator_utils.h"

namespace asm8086 {

namespace {

bool isDirectAddrMode(Mod mod, u8 rm) {
    return mod == Mod::MEMORY_NO_DISPLACEMENT && rm == 0b110;
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
            inst.rm = inst.reg; // special case
            inst.type = InstType::MOV;
            break;
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:
            inst.operands = inst.d ? Operands::Register_Memory : Operands::Memory_Register;
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
            inst.operands = isEffectiveAddrCalc(inst.mod) ? Operands::Memory_SegReg : Operands::Register16_SegReg;
            inst.type = InstType::MOV;
            break;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:
            inst.operands = isEffectiveAddrCalc(inst.mod) ? Operands::SegReg_Memory16 : Operands::SegReg_Register16;
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
            inst.operands = inst.d ? Operands::Register_Memory : Operands::Memory_Register;
            inst.type = InstType::ADD;
            break;
        case ADD_IMM_TO_ACC:
            inst.operands = Operands::Accumulator_Immediate;
            inst.type = InstType::ADD;
            break;

        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT:
            inst.operands = inst.d ? Operands::Register_Memory : Operands::Memory_Register;
            inst.type = InstType::SUB;
            break;
        case SUB_IMM_FROM_ACC:
            inst.operands = Operands::Accumulator_Immediate;
            inst.type = InstType::SUB;
            break;

        case CMP_REG_OR_MEM_WITH_REG:
            inst.operands = inst.d ? Operands::Register_Memory : Operands::Memory_Register;
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

void appendImmediate(core::str_builder<>& sb, u16 i) {
    char ncptr[8] = {};
    core::int_to_cptr(u32(i), ncptr);
    sb.append(ncptr);
}

GUARD_FN_TYPE_DEDUCTION(appendImmediate);

void appendImmediateSignedWord(core::str_builder<>& sb, i16 i, bool explictSign = true) {
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

void appendImmediateSignedByte(core::str_builder<>& sb, i8 i, bool explictSign = true) {
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

                                        // 000  001   010   011   100   101   110   111
constexpr const char* reg8bitTable[]  = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" }; // w = 0
constexpr const char* reg16bitTable[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" }; // w = 1

                                        //   000       001         010        011     100   101   110   111
constexpr const char* regDispTable[]  = { "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx" }; // w = 0

void appendReg(core::str_builder<>& sb, u8 reg, bool w) {
    sb.append(w ? reg16bitTable[reg] : reg8bitTable[reg]);
}

void appendRegDisp(core::str_builder<>& sb, u8 reg, u16 disp) {
    sb.append(regDispTable[reg]);
    if (disp != 0) {
        sb.append(" ");
        appendImmediateSignedWord(sb, i16(disp), true);
    }
}

void appendMemory(core::str_builder<>& sb, u8 w, u8 rm, u16 disp, bool isCalc) {
    if (!isCalc) {
        appendReg(sb, rm, w);
    }
    else {
        sb.append("[");
        appendRegDisp(sb, rm, disp);
        sb.append("]");
    }
}

void encodeInstruction(core::str_builder<>& sb,
                       const Instruction& inst,
                       const core::arr<JmpLabel>& jmpLabels,
                       addr_size byteIdx) {
    sb.append(instTypeToCptr(inst.type));
    sb.append(" ");

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

    u16 data = u16FromLowAndHi((w == 1), dataLow, dataHigh);
    u16 disp = 0;
    if (isDirect) {
        disp = isCalc ? u16FromLowAndHi((w == 1), dispLow, dispHigh) : 0;
    }
    else {
        disp = isCalc ? u16FromLowAndHi(is16bitDisplacement(mod), dispLow, dispHigh) : 0;
    }

    auto appendMemoryAccumulator = [&]() {
        appendReg(sb, reg, w);
        sb.append(", ");
        sb.append("["); appendImmediateSignedWord(sb, data, false); sb.append("]");
    };
    auto appendMemoryRegister = [&]() {
        appendMemory(sb, w, rm, disp, isCalc);
        sb.append(", ");
        appendReg(sb, reg, w);
    };
    auto appendMemoryImmediate = [&]() {
        sb.append(w ? "word " : "byte ");
        if (isDirect) {
            sb.append("["); appendImmediateSignedWord(sb, disp, false); sb.append("]");
        }
        else {
            appendMemory(sb, w, rm, disp, isCalc);
        }
        sb.append(", ");
        appendImmediateSignedWord(sb, data, false);
    };
    auto appendRegisterMemory = [&]() {
        appendReg(sb, reg, w);
        sb.append(", ");
        if (isDirect) {
            sb.append("["); appendImmediateSignedWord(sb, disp, false); sb.append("]");
        }
        else {
            appendMemory(sb, w, rm, disp, isCalc);
        }
    };
    auto appendRegisterImmediate = [&]() {
        appendReg(sb, rm, w);
        sb.append(", ");
        appendImmediateSignedWord(sb, data, false);
    };
    auto appendAccumulatorMemory = [&]() {
        sb.append("["); appendImmediateSignedWord(sb, data, false); sb.append("]");
        sb.append(", ");
        appendReg(sb, rm, w);
    };
    auto appendAccumulatorImmediate = [&]() {
        appendReg(sb, rm, w);
        sb.append(", ");
        appendImmediateSignedWord(sb, data, false);
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
            // TODO: I should makeup my mind on how long a jump is allowed. Is there a point to use 64 bit nubmers, if yes,
            //       then there should be a function to append them.
            appendImmediate(sb, u16(jmpLabels[jmpidx].labelIdx));
        }
    };

    switch (operands) {
        case Operands::Memory_Accumulator:    appendMemoryAccumulator();     break;
        case Operands::Memory_Register:       appendMemoryRegister();        break;
        case Operands::Memory_Immediate:      appendMemoryImmediate();       break;
        case Operands::Register_Memory:       appendRegisterMemory();        break;
        case Operands::Register_Immediate:    appendRegisterImmediate();     break;
        case Operands::Accumulator_Memory:    appendAccumulatorMemory();     break;
        case Operands::Accumulator_Immediate: appendAccumulatorImmediate();  break;
        case Operands::ShortLabel:            appendShortLabel();            break;
        case Operands::Register_Register:                                    [[fallthrough]];
        case Operands::SegReg_Register16:                                    [[fallthrough]];
        case Operands::SegReg_Memory16:                                      [[fallthrough]];
        case Operands::Register16_SegReg:                                    [[fallthrough]];
        case Operands::Memory_SegReg:                                        [[fallthrough]];
        case Operands::None:                  sb.append("(encoding faild)"); break;
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
                appendImmediate(asmOut, u64(ctx.jmpLabels[jmpidx].labelIdx));
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
        encodeInstruction(asmOut, inst, ctx.jmpLabels, byteIdx);
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
        case RegisterType::CS:    return "cs";
        case RegisterType::DS:    return "ds";
        case RegisterType::SS:    return "ss";
        case RegisterType::ES:    return "es";
        case RegisterType::IP:    return "ip";
        case RegisterType::FLAGS: return "flags";
        default:                  return "invalid register";
    }
}

namespace {

void setRegister(EmulationContext& ctx, const RegisterType& rtype, u16 value) {
    ctx.registers[i32(rtype)].value = value;
}

void executeMov(EmulationContext& ctx, const Instruction& inst) {
    if (inst.operands == Operands::Register_Immediate) {
        Register& destReg = ctx.registers[i32(inst.reg)];
        u16 immValue = inst.w ? ((u16(inst.data[1]) << 8) | u16(inst.data[0])) : (inst.data[0]);

        u16 prev = destReg.value;
        destReg.value = immValue;

        if (ctx.options & EmulationOptionFlags::EMU_FLAG_VERBOSE) {
            fmt::print("{} {}, {} ; {}:{:#0x}->{:#0x}\n",
                        instTypeToCptr(inst.type), regTypeToCptr(destReg.type), immValue,
                        regTypeToCptr(destReg.type), prev, destReg.value);
        }
    }
    else if (inst.operands == Operands::Memory_Register) {
        Register& destReg = ctx.registers[i32(inst.rm)];
        Register& srcReg = ctx.registers[i32(inst.reg)];

        u16 prev = srcReg.value;
        destReg.value = srcReg.value;

        if (ctx.options & EmulationOptionFlags::EMU_FLAG_VERBOSE) {
            fmt::print("{} {}, {} ; {}:{:#0x}->{:#0x}\n",
                        instTypeToCptr(inst.type), regTypeToCptr(destReg.type), srcReg.value,
                        regTypeToCptr(srcReg.type), prev, destReg.value);
        }
    }
}

} // namespace

void emulate(EmulationContext& ctx) {
    for (addr_size i = 0; i < ctx.instructions.len(); i++) {
        auto inst = ctx.instructions[i];
        if (isDisplacementMode(inst.mod)) Panic(false, "Can't handle displacement mode yet.");

        switch (inst.type) {
            case InstType::MOV: executeMov(ctx, inst); break;
            default:
                Panic(false, "Instruction type not supported yet.");
                break;
        }
    }
}

} // namespace asm8086
