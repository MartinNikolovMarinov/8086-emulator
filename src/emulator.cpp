#include "emulator.h"
#include "emulator_utils.h"

namespace asm8086 {

namespace {

bool isDirectAddrMode(Mod mod, u8 rm) {
    return mod == MOD_MEMORY_NO_DISPLACEMENT && rm == 0b110;
}

bool isDisplacementMode(Mod mod) {
    return mod == MOD_MEMORY_8_BIT_DISPLACEMENT ||
           mod == MOD_MEMORY_16_BIT_DISPLACEMENT;
}

bool isEffectiveAddrCalc(Mod mod) {
    return mod != MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT;
}

bool is8bitDisplacement(Mod mod) {
    return mod == MOD_MEMORY_8_BIT_DISPLACEMENT;
}

bool is16bitDisplacement(Mod mod) {
    return mod == MOD_MEMORY_16_BIT_DISPLACEMENT;
}

Instruction decodeInstruction(core::arr<u8>& bytes, DecodingContext& ctx) {
    auto decodeFromDisplacements = [&](auto& bytes, i32 idx, const FieldDisplacements& fd, Instruction& inst) {
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
        if (fd.disp1.byteIdx > 0 && inst.mod == MOD_MEMORY_8_BIT_DISPLACEMENT) {
            inst.disp[0] = bytes[idx + ibc + 1];
            ibc++;
        }
        else if (fd.disp1.byteIdx > 0 &&
                 (inst.mod == MOD_MEMORY_16_BIT_DISPLACEMENT ||
                  isDirectAddrMode(inst.mod, inst.rm))
        ) {
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

    auto& jmpLabels = ctx.jmpLabels;

    ptr_size idx = ctx.idx;
    Opcode opcode = opcodeDecode(bytes[idx]);
    auto fd = getFieldDisplacements(opcode);

    Instruction inst = {};
    inst.opcode = opcode;
    decodeFromDisplacements(bytes, idx, fd, inst);

    switch (inst.opcode) {
        case MOV_IMM_TO_REG:
        {
            inst.operands = Operands::Register_Immediate;
            inst.rm = inst.reg; // special case
            inst.type = InstType::MOV;
            break;
        }
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:
        {
            inst.operands = inst.d ? Operands::Register_Memory : Operands::Memory_Register;
            inst.type = InstType::MOV;
            break;
        }
        case MOV_MEM_TO_ACC:
        {
            inst.operands = Operands::Memory_Accumulator;
            inst.type = InstType::MOV;
            break;
        }
        case MOV_ACC_TO_MEM:
        {
            inst.operands = Operands::Accumulator_Memory;
            inst.type = InstType::MOV;
            break;
        }
        case MOV_IMM_TO_REG_OR_MEM:
        {
            inst.operands = isEffectiveAddrCalc(inst.mod) ? Operands::Memory_Immediate : Operands::Register_Immediate;
            inst.type = InstType::MOV;
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
        {
            inst.operands = inst.d ? Operands::Register_Memory : Operands::Memory_Register;
            inst.type = InstType::ADD;
            break;
        }
        case ADD_IMM_TO_ACC:
        {
            inst.operands = Operands::Accumulator_Immediate;
            inst.type = InstType::ADD;
            break;
        }

        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT:
        {
            inst.operands = inst.d ? Operands::Register_Memory : Operands::Memory_Register;
            inst.type = InstType::SUB;
            break;
        }
        case SUB_IMM_FROM_ACC:
        {
            inst.operands = Operands::Accumulator_Immediate;
            inst.type = InstType::SUB;
            break;
        }

        case CMP_REG_OR_MEM_WITH_REG:
        {
            inst.operands = inst.d ? Operands::Register_Memory : Operands::Memory_Register;
            inst.type = InstType::CMP;
            break;
        }
        case CMP_IMM_WITH_ACC:
        {
            inst.operands = Operands::Accumulator_Immediate;
            inst.type = InstType::CMP;
            break;
        }

        case JNEZ_ON_NOT_EQ_NOR_ZERO:
        {
            inst.operands = Operands::ShortLabel;
            inst.type = InstType::JNZ;
            ptr_size diff = 0;
            i8 shortJmpDiff = i8(inst.data[0]);
            safeCastSignedInt(shortJmpDiff, diff);
            ptr_size byteOff = ptr_size(idx) + ptr_size(inst.byteCount) + ptr_size(diff);
            JmpLabel jmpLabel = { byteOff, jmpLabels.len() };
            core::appendUnique(jmpLabels, jmpLabel, [&](auto& x) -> bool { return x.byteOffset == byteOff; });
            break;
        }
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

namespace {

const char* instTypeToCptr(InstType t) {
    switch (t) {
        case InstType::MOV:     return "mov";
        case InstType::ADD:     return "add";
        case InstType::SUB:     return "sub";
        case InstType::CMP:     return "cmp";
        case InstType::JNZ:     return "jnz";
        case InstType::JNE:     return "jne";
        case InstType::UNKNOWN: return "unknown";
        case InstType::SENTINEL: break;
    }
    return "invalid";
}

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
                       ptr_size byteIdx) {
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
        disp = isCalc ? u16FromLowAndHi((mod == MOD_MEMORY_16_BIT_DISPLACEMENT), dispLow, dispHigh) : 0;
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
        ptr_size byteOff = byteIdx + shotJmpOffset;
        i64 jmpidx = core::find(jmpLabels, [&](auto& el, ptr_size) -> bool { return el.byteOffset == byteOff; });
        if (jmpidx == -1) {
            sb.append("(failed to decode label)");
        }
        else {
            sb.append("label_");
            // TODO: I should makeup my mind on how long a jump is allowed. Is there a point to use 64 bit nubmers, if yes,
            //       then there should be a function to append them.
            appendImmediate(sb, u16(jmpLabels[jmpidx].labelIdx));
            // sb.append(" ; ");
            // appendImmediateSignedByte(sb, shotJmpOffset, false);
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
        case Operands::SetReg_Memory16:                                      [[fallthrough]];
        case Operands::Register16_SegReg:                                    [[fallthrough]];
        case Operands::Memory_SegReg:                                        [[fallthrough]];
        case Operands::None:                  sb.append("(encoding faild)"); break;
    }
}

}

void encodeAsm8086(core::str_builder<>& asmOut, const DecodingContext& ctx) {
    ptr_size byteIdx = 0;
    for (ptr_size i = 0; i <= ctx.instructions.len(); i++) {
        // Insert the label before the instruction.
        {
            i64 jmpidx = core::find(ctx.jmpLabels, [&](auto& el, ptr_size) -> bool {
                return el.byteOffset == byteIdx;
            });
            if (jmpidx != -1) {
                asmOut.append("label_");
                appendImmediate(asmOut, u64(ctx.jmpLabels[jmpidx].labelIdx));
                asmOut.append(":");
                asmOut.append("\n");
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

} // namespace asm8086
