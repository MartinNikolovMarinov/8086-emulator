#include <emulator.h>
#include <decoder.h>
#include <utils.h>
#include <logger.h>

namespace asm8086 {

const char* regTypeToCptr(const RegisterType& rtype) {
    switch (rtype) {
        case RegisterType::AX:       return "ax";
        case RegisterType::CX:       return "cx";
        case RegisterType::DX:       return "dx";
        case RegisterType::BX:       return "bx";
        case RegisterType::SP:       return "sp";
        case RegisterType::BP:       return "bp";
        case RegisterType::SI:       return "si";
        case RegisterType::DI:       return "di";
        case RegisterType::ES:       return "es";
        case RegisterType::CS:       return "cs";
        case RegisterType::SS:       return "ss";
        case RegisterType::DS:       return "ds";
        case RegisterType::IP:       return "ip";
        case RegisterType::FLAGS:    return "flags";
        case RegisterType::SENTINEL: break;
    }
    return "invalid register";
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

EmulationContext createEmulationCtx(core::Arr<Instruction>&& instructions, EmulationOpts options) {
    EmulationContext ctx;
    ctx.instructions = core::move(instructions);
    ctx.emuOpts = options;
    core::memset(ctx.memory, 0, EmulationContext::MEMORY_SIZE);
    for (addr_size i = 0; i < addr_size(RegisterType::SENTINEL); i++) {
        auto& reg = ctx.registers[i];
        reg.type = RegisterType(i);
        reg.value = 0;
    }
    return ctx;
}

namespace {

enum struct InstClassification : u8 {
    None,
    DataTransfer,
    Arithmentic,
    Logical,
    StringManipulation,
    ControlTransfer,
    ProcessorControl,

    SENTINEL
};

InstClassification getClassification(InstType itype) {
    switch (itype) {
        case InstType::MOV:
            return InstClassification::DataTransfer;

        case InstType::CMP: [[fallthrough]];
        case InstType::SUB: [[fallthrough]];
        case InstType::ADD:
            return InstClassification::Arithmentic;

        case InstType::JE:     [[fallthrough]];
        case InstType::JZ:     [[fallthrough]];
        case InstType::JL:     [[fallthrough]];
        case InstType::JNGE:   [[fallthrough]];
        case InstType::JLE:    [[fallthrough]];
        case InstType::JNG:    [[fallthrough]];
        case InstType::JB:     [[fallthrough]];
        case InstType::JNAE:   [[fallthrough]];
        case InstType::JBE:    [[fallthrough]];
        case InstType::JNA:    [[fallthrough]];
        case InstType::JP:     [[fallthrough]];
        case InstType::JPE:    [[fallthrough]];
        case InstType::JO:     [[fallthrough]];
        case InstType::JS:     [[fallthrough]];
        case InstType::JNE:    [[fallthrough]];
        case InstType::JNZ:    [[fallthrough]];
        case InstType::JNL:    [[fallthrough]];
        case InstType::JGE:    [[fallthrough]];
        case InstType::JNLE:   [[fallthrough]];
        case InstType::JG:     [[fallthrough]];
        case InstType::JNB:    [[fallthrough]];
        case InstType::JAE:    [[fallthrough]];
        case InstType::JNBE:   [[fallthrough]];
        case InstType::JA:     [[fallthrough]];
        case InstType::JNP:    [[fallthrough]];
        case InstType::JPO:    [[fallthrough]];
        case InstType::JNO:    [[fallthrough]];
        case InstType::JNS:    [[fallthrough]];
        case InstType::LOOP:   [[fallthrough]];
        case InstType::LOOPE:  [[fallthrough]];
        case InstType::LOOPZ:  [[fallthrough]];
        case InstType::LOOPNE: [[fallthrough]];
        case InstType::LOOPNZ: [[fallthrough]];
        case InstType::JCXZ:
            return InstClassification::ControlTransfer;

        case InstType::UNKNOWN:  [[fallthrough]];
        case InstType::SENTINEL: break;
    }

    return InstClassification::None;
}

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

addr_off calcMemoryAddress(const EmulationContext& ctx, const Instruction& inst) {
    bool isDirect = isDirectAddrMode(inst.mod, inst.rm);
    u8 dispLow = inst.disp[0];
    u8 dispHi = inst.disp[1];
    addr_off addr = 0;

    if (!isDirect) {
        switch (inst.rm) {
            case 0b000:
            {
                // [BX + SI]
                i16 bx = i16(ctx.registers[i32(RegisterType::BX)].value);
                i16 si = i16(ctx.registers[i32(RegisterType::SI)].value);
                addr += bx + si;
                break;
            }
            case 0b001:
            {
                // [BX + DI]
                i16 bx = i16(ctx.registers[i32(RegisterType::BX)].value);
                i16 di = i16(ctx.registers[i32(RegisterType::DI)].value);
                addr += bx + di;
                break;
            }
            case 0b010:
            {
                // [BP + SI]
                i16 bp = i16(ctx.registers[i32(RegisterType::BP)].value);
                i16 si = i16(ctx.registers[i32(RegisterType::SI)].value);
                addr += bp + si;
                break;
            }
            case 0b011:
            {
                // [BP + DI]
                i16 bp = i16(ctx.registers[i32(RegisterType::BP)].value);
                i16 di = i16(ctx.registers[i32(RegisterType::DI)].value);
                addr += bp + di;
                break;
            }
            case 0b100:
            {
                // [SI]
                i16 si = i16(ctx.registers[i32(RegisterType::SI)].value);
                addr += si;
                break;
            }
            case 0b101:
            {
                // [DI]
                i16 di = i16(ctx.registers[i32(RegisterType::DI)].value);
                addr += di;
                break;
            }
            case 0b110:
            {
                // [BP]
                i16 bp = i16(ctx.registers[i32(RegisterType::BP)].value);
                addr += bp;
                break;
            }
            case 0b111:
            {
                // [BX]
                i16 bx = i16(ctx.registers[i32(RegisterType::BX)].value);
                addr += bx;
                break;
            }
        }
    }

    if (is8bitDisplacement(inst.mod)) {
        // [... + disp8]
        addr += i8(dispLow);
    }
    else if (is16bitDisplacement(inst.mod) || isDirect) {
        // [... + disp16]
        i16 disp16 = i16(combineWord(dispLow, dispHi));
        addr += disp16;
    }

    // User bug:
    Panic(addr >= 0 && addr_size(addr) < EmulationContext::MEMORY_SIZE - 1, "Indexing memory out of bounds.");
    return addr;
}

struct Dest {
    bool isWord;
    bool isLow;
    u16* target;
};

struct Source {
    u8 low;
    u8 hi;
    bool isWord;
    bool isLow;
};

void emulateMov(Dest& dst, Source& src) {
    u16 original = *dst.target;
    u16 next;

    if (dst.isWord) {
        next = combineWord(src.low, src.hi);
    }
    else {
        u8 srcVal = src.isLow ? src.low : src.hi;
        if (dst.isLow) {
            next = combineWord(srcVal, highPart(original));
        }
        else {
            next = combineWord(lowPart(original), srcVal);
        }
    }

    *dst.target = next;
}

void emulateAdd(Dest& dst, Source& src, Register& flags) {
    bool signFlag, zeroFlag, carryFlag, overflowFlag, parityFlag, auxCarryFlag;
    u16 original = *dst.target;
    u16 next;

    if (dst.isWord) {
        u16 srcVal;
        if (src.isWord) {
            srcVal = combineWord(src.low, src.hi);
        }
        else {
            i16 tmp = 0;
            safeCastSignedInt(i8(src.low), tmp);
            srcVal = u16(tmp);
        }
        next = original + srcVal;
        signFlag = i16(next) < 0;
        zeroFlag = next == 0;
        carryFlag = next < original;
        overflowFlag = isSignedBitSet(srcVal) == isSignedBitSet(original) &&
                       isSignedBitSet(srcVal) != isSignedBitSet(next);
        auxCarryFlag = ((original & 0xF) + (srcVal & 0xF)) > 0xF;
        i32 setBitsCount = i32(core::intrin_numberOfSetBits(u32(lowPart(next))));
        parityFlag = (setBitsCount & 0x1) == 0;
    }
    else {
        u8 srcVal = src.isLow ? src.low : src.hi;
        if (dst.isLow) {
            u8 dstLow = lowPart(original);
            dstLow += srcVal;
            next = combineWord(dstLow, highPart(original));
            signFlag = i8(dstLow) < 0;
            zeroFlag = dstLow == 0;
            carryFlag = dstLow < lowPart(original);
            overflowFlag = isSignedBitSet(srcVal) == isSignedBitSet(lowPart(original)) &&
                           isSignedBitSet(srcVal) != isSignedBitSet(dstLow);
            auxCarryFlag = ((lowPart(original) & 0xF) + (srcVal & 0xF)) > 0xF;
            i32 setBitsCount = i32(core::intrin_numberOfSetBits(u32(lowPart(next))));
            parityFlag = (setBitsCount & 0x1) == 0;
        }
        else {
            u8 dstHigh = highPart(original);
            dstHigh += srcVal;
            next = combineWord(lowPart(original), dstHigh);
            signFlag = i8(dstHigh) < 0;
            zeroFlag = dstHigh == 0;
            carryFlag = dstHigh < highPart(original);
            overflowFlag = isSignedBitSet(srcVal) == isSignedBitSet(highPart(original)) &&
                           isSignedBitSet(srcVal) != isSignedBitSet(dstHigh);
            auxCarryFlag = ((highPart(original) & 0x0F) + (srcVal & 0x0F)) > 0xF;
            i32 setBitsCount = i32(core::intrin_numberOfSetBits(u32(highPart(next))));
            parityFlag = (setBitsCount & 0x1) == 0;
        }

    }

    *dst.target = next;

    setFlag(flags, CPU_FLAG_SIGN_FLAG, signFlag);
    setFlag(flags, CPU_FLAG_ZERO_FLAG, zeroFlag);
    setFlag(flags, CPU_FLAG_CARRY_FLAG, carryFlag);
    setFlag(flags, CPU_FLAG_OVERFLOW_FLAG, overflowFlag);
    setFlag(flags, CPU_FLAG_PARITY_FLAG, parityFlag);
    setFlag(flags, CPU_FLAG_AUX_CARRY_FLAG, auxCarryFlag);
}

void emulateSub(Dest& dst, Source& src, Register& flags) {
    bool signFlag, zeroFlag, carryFlag, overflowFlag, parityFlag, auxCarryFlag;
    u16 original = *dst.target;
    u16 next;

    if (dst.isWord) {
        u16 srcVal;
        if (src.isWord) {
            srcVal = combineWord(src.low, src.hi);
        }
        else {
            i16 tmp = 0;
            safeCastSignedInt(i8(src.low), tmp);
            srcVal = u16(tmp);
        }
        next = original - srcVal;
        signFlag = i16(next) < 0;
        zeroFlag = next == 0;
        carryFlag = original < next;
        overflowFlag = (isSignedBitSet(original) != isSignedBitSet(srcVal)) &&
                       (isSignedBitSet(original) != isSignedBitSet(next));
        auxCarryFlag = ((original & 0xF) - (srcVal & 0xF)) < 0;
        i32 setBitsCount = i32(core::intrin_numberOfSetBits(u32(lowPart(next))));
        parityFlag = (setBitsCount & 0x1) == 0;
    }
    else {
        u8 srcVal = src.isLow ? src.low : src.hi;
        if (dst.isLow) {
            u8 dstLow = lowPart(original);
            dstLow -= srcVal;
            next = combineWord(dstLow, highPart(original));
            signFlag = i8(dstLow) < 0;
            zeroFlag = dstLow == 0;
            carryFlag = lowPart(original) < dstLow;
            overflowFlag = (isSignedBitSet(lowPart(original)) != isSignedBitSet(srcVal)) &&
                           (isSignedBitSet(lowPart(original)) != isSignedBitSet(dstLow));
            auxCarryFlag = ((lowPart(original) & 0xF) - (srcVal & 0xF)) < 0;
            i32 setBitsCount = i32(core::intrin_numberOfSetBits(u32(lowPart(next))));
            parityFlag = (setBitsCount & 0x1) == 0;
        }
        else {
            u8 dstHigh = highPart(original);
            dstHigh -= srcVal;
            next = combineWord(lowPart(original), dstHigh);
            signFlag = i8(dstHigh) < 0;
            zeroFlag = dstHigh == 0;
            carryFlag = highPart(original) < dstHigh;
            overflowFlag = (isSignedBitSet(highPart(original)) != isSignedBitSet(srcVal)) &&
                           (isSignedBitSet(highPart(original)) != isSignedBitSet(dstHigh));
            auxCarryFlag = ((highPart(original) & 0x0F) - (srcVal & 0x0F)) < 0;
            i32 setBitsCount = i32(core::intrin_numberOfSetBits(u32(highPart(next))));
            parityFlag = (setBitsCount & 0x1) == 0;
        }

    }

    *dst.target = next;

    setFlag(flags, CPU_FLAG_SIGN_FLAG, signFlag);
    setFlag(flags, CPU_FLAG_ZERO_FLAG, zeroFlag);
    setFlag(flags, CPU_FLAG_CARRY_FLAG, carryFlag);
    setFlag(flags, CPU_FLAG_OVERFLOW_FLAG, overflowFlag);
    setFlag(flags, CPU_FLAG_PARITY_FLAG, parityFlag);
    setFlag(flags, CPU_FLAG_AUX_CARRY_FLAG, auxCarryFlag);
}

void emulateNext(EmulationContext& ctx, const Instruction& inst) {
    Register* destRegister = nullptr;
    u16* destMemoryAddress = nullptr;
    Dest dst = {};
    dst.isWord = (inst.w == 1);
    Source src = {};

    switch (inst.operands) {
        case Operands::Register_Immediate:
        {
            // Set destination
            destRegister = getRegister(ctx, inst.rm, dst.isWord, false);
            dst.target = &destRegister->value;
            dst.isLow = isLowRegister(inst.rm);
            // Set source
            src.low = inst.data[0];
            src.hi = inst.data[1];
            src.isLow = true;
            src.isWord = inst.s ? false : (inst.w == 1);
            break;
        }
        case Operands::Register_Register:
        {
            // Set destination
            destRegister = getRegister(ctx, inst.rm, dst.isWord, false);
            dst.target = &destRegister->value;
            dst.isLow = isLowRegister(inst.rm);
            // Set source
            Register* rsrc = getRegister(ctx, inst.reg, dst.isWord, false);
            src.low = lowPart(rsrc->value);
            src.hi = highPart(rsrc->value);
            src.isLow = isLowRegister(inst.reg);
            src.isWord = dst.isWord;
            break;
        }
        case Operands::Register16_SegReg:
        {
            // Set destination
            destRegister= getRegister(ctx, inst.reg, true, true);
            dst.target = &destRegister->value;
            dst.isLow = false;
            dst.isWord = true;
            // Set source
            Register* rsrc = getRegister(ctx, inst.rm, true, false);
            src.low = lowPart(rsrc->value);
            src.hi = highPart(rsrc->value);
            src.isLow = true;
            src.isWord = true;
            break;
        }
        case Operands::SegReg_Register16:
        {
            // Set destination
            destRegister = getRegister(ctx, inst.rm, true, false);
            dst.target = &destRegister->value;
            dst.isLow = false;
            dst.isWord = true;
            // Set source
            Register* rsrc = getRegister(ctx, inst.reg, true, true);
            src.low = lowPart(rsrc->value);
            src.hi = highPart(rsrc->value);
            src.isLow = true;
            src.isWord = true;
            break;
        }

        case Operands::Memory_Register:
        {
            // Set destination
            destRegister = getRegister(ctx, inst.reg, dst.isWord, false);
            dst.target = &destRegister->value;
            dst.isLow = isLowRegister(inst.reg);
            // Set source
            addr_off effectiveAddr = calcMemoryAddress(ctx, inst);
            u16* srcMemoryAddress = reinterpret_cast<u16*>(ctx.memory + effectiveAddr);
            src.low = *reinterpret_cast<u8*>(srcMemoryAddress);
            src.hi = *(reinterpret_cast<u8*>(srcMemoryAddress) + 1);
            src.isLow = true;
            src.isWord = inst.s ? false : (inst.w == 1);
            break;
        }

        case Operands::Register_Memory: {
            // Set destination
            addr_off effectiveAddr = calcMemoryAddress(ctx, inst);
            destMemoryAddress = reinterpret_cast<u16*>(ctx.memory + effectiveAddr);
            dst.target = destMemoryAddress;
            dst.isLow = dst.isWord;
            // Set source
            Register* rsrc = getRegister(ctx, inst.reg, true, false);
            src.low = lowPart(rsrc->value);
            src.hi = highPart(rsrc->value);
            src.isLow = true;
            src.isWord = true;
            break;
        }

        case Operands::Memory_Immediate: {
            // Set destination
            addr_off effectiveAddr = calcMemoryAddress(ctx, inst);
            destMemoryAddress = reinterpret_cast<u16*>(ctx.memory + effectiveAddr);
            dst.target = destMemoryAddress;
            dst.isLow = dst.isWord;
            // Set source
            src.isWord = inst.s ? false : (inst.w == 1);
            src.low = inst.data[0];
            src.hi = inst.data[1];
            if (!src.isWord) {
                // This handles targetting the exact byte in memory when the instruction is a byte sized.
                src.hi = src.low;
                dst.isLow = true;
                src.isLow = false;
            }
            break;
        };

        case Operands::Accumulator_Immediate:
        {
            // Set destination
            destRegister = &ctx.registers[i32(RegisterType::AX)];
            dst.target = &destRegister->value;
            dst.isLow = true;
            // Set source
            src.isWord = dst.isWord;
            src.isLow = dst.isLow;
            src.low = inst.data[0];
            src.hi = inst.data[1];
            break;
        }
        case Operands::Memory_Accumulator:
        {
            // Set destination
            destRegister = &ctx.registers[i32(RegisterType::AX)];
            dst.target = &destRegister->value;
            dst.isLow = true;
            // Set source
            Instruction instCpy = inst;
            {
                // This instruction is a bit special. The data is addr and in this case it should be used as the
                // displacement for the effective memory calculation. Remember that here the mode is not set!
                instCpy.mod = Mod::MEMORY_16_BIT_DISPLACEMENT;
                instCpy.disp[0] = inst.data[0];
                instCpy.disp[1] = inst.data[1];
            }
            addr_off effectiveAddr = calcMemoryAddress(ctx, instCpy);
            u16* srcMemoryAddress = reinterpret_cast<u16*>(ctx.memory + effectiveAddr);
            src.low = *reinterpret_cast<u8*>(srcMemoryAddress);
            src.hi = *(reinterpret_cast<u8*>(srcMemoryAddress) + 1);
            src.isLow = dst.isLow;
            src.isWord = dst.isWord;
            break;
        }
        case Operands::Accumulator_Memory:
        {
            // Set destination
            Instruction instCpy = inst;
            {
                // Same reason as per Memory_Accumulator
                instCpy.mod = Mod::MEMORY_16_BIT_DISPLACEMENT;
                instCpy.disp[0] = inst.data[0];
                instCpy.disp[1] = inst.data[1];
            }
            addr_off effectiveAddr = calcMemoryAddress(ctx, instCpy);
            destMemoryAddress = reinterpret_cast<u16*>(ctx.memory + effectiveAddr);
            dst.target = destMemoryAddress;
            dst.isLow = true;
            // Set source
            Register& accReg = ctx.registers[i32(RegisterType::AX)];
            src.isWord = dst.isWord;
            src.isLow = dst.isLow;
            src.low = lowPart(accReg.value);
            src.hi = highPart(accReg.value);
            break;
        }

        case Operands::ShortLabel: break; // nothing to do

        case Operands::SegReg_Memory16:            [[fallthrough]];
        case Operands::Memory_SegReg:              [[fallthrough]];
        case Operands::None:                       [[fallthrough]];
        case Operands::SENTINEL:                   Assert(false, "Unsupported instruction operands."); return;
    }

    InstClassification cmdType = getClassification(inst.type);

    // Sanity checks:
    using IC = InstClassification;
    Assert(cmdType != IC::None, "Failed to classify command.");
    if (cmdType == IC::DataTransfer || cmdType == IC::Arithmentic || cmdType == IC::Logical) {
        Assert(dst.target, "Failed to set destination for instruction that requires it.");
    }

    u16 old = dst.target ? *dst.target : 0;
    Register& ip = ctx.registers[i32(RegisterType::IP)];
    i16 deltaIp = 0;

    switch (inst.type) {
        case InstType::MOV:
            emulateMov(dst, src);
            break;
        case InstType::ADD:
            emulateAdd(dst, src, getFlagsRegister(ctx));
            break;
        case InstType::SUB:
            emulateSub(dst, src, getFlagsRegister(ctx));
            break;
        case InstType::CMP:
            emulateSub(dst, src, getFlagsRegister(ctx));
            *dst.target = old; // cmp is the same as sub, but doesn't write to dst
            break;
        case InstType::JNZ:
        case InstType::JNE:
        {
            Register& flags = getFlagsRegister(ctx);
            if (isFlagSet(flags, Flags::CPU_FLAG_ZERO_FLAG) == false) {
                deltaIp += i8(inst.data[0]);
            }
            break;
        }
        case InstType::JZ:
        case InstType::JE:
        {
            Register& flags = getFlagsRegister(ctx);
            if (isFlagSet(flags, Flags::CPU_FLAG_ZERO_FLAG) == true) {
                deltaIp += i8(inst.data[0]);
            }
            break;
        }
        case InstType::JP:
        case InstType::JPE:
        {
            Register& flags = getFlagsRegister(ctx);
            if (isFlagSet(flags, Flags::CPU_FLAG_PARITY_FLAG) == true) {
                deltaIp += i8(inst.data[0]);
            }
            break;
        }
        case InstType::JB:
        case InstType::JNAE:
        {
            Register& flags = getFlagsRegister(ctx);
            if (isFlagSet(flags, Flags::CPU_FLAG_CARRY_FLAG) == true) {
                deltaIp += i8(inst.data[0]);
            }
            break;
        }
        case InstType::LOOPNZ:
        case InstType::LOOPNE:
        {
            Register& cx = ctx.registers[i32(RegisterType::CX)];
            cx.value--; // Decrement CX. NOTE: Interestingly, this should not set any flags!
            Register& flags = getFlagsRegister(ctx);
            if (cx.value != 0 && isFlagSet(flags, Flags::CPU_FLAG_ZERO_FLAG) == false) {
                deltaIp += i8(inst.data[0]);
            }
            break;
        }
        case InstType::LOOP:
        {
            Register& cx = ctx.registers[i32(RegisterType::CX)];
            cx.value--; // Decrement CX. NOTE: Interestingly, this should not set any flags!
            if (cx.value != 0) {
                deltaIp += i8(inst.data[0]);
            }
            break;
        }

        case InstType::JL:       [[fallthrough]];
        case InstType::JNGE:     [[fallthrough]];
        case InstType::JLE:      [[fallthrough]];
        case InstType::JNG:      [[fallthrough]];
        case InstType::JBE:      [[fallthrough]];
        case InstType::JNA:      [[fallthrough]];
        case InstType::JO:       [[fallthrough]];
        case InstType::JS:       [[fallthrough]];
        case InstType::JNL:      [[fallthrough]];
        case InstType::JGE:      [[fallthrough]];
        case InstType::JNLE:     [[fallthrough]];
        case InstType::JG:       [[fallthrough]];
        case InstType::JNB:      [[fallthrough]];
        case InstType::JAE:      [[fallthrough]];
        case InstType::JNBE:     [[fallthrough]];
        case InstType::JA:       [[fallthrough]];
        case InstType::JNP:      [[fallthrough]];
        case InstType::JPO:      [[fallthrough]];
        case InstType::JNO:      [[fallthrough]];
        case InstType::JNS:      [[fallthrough]];
        case InstType::LOOPE:    [[fallthrough]];
        case InstType::LOOPZ:    [[fallthrough]];
        case InstType::JCXZ:     [[fallthrough]];
        case InstType::SENTINEL: [[fallthrough]];
        case InstType::UNKNOWN:  Assert(false, "Instruction not supported for emulation."); return;
    }

    u16 nextIp = u16(ip.value + deltaIp + inst.byteCount);

    if (ctx.emuOpts & EmulationOpts::EMU_OPT_VERBOSE) {
        static i64 tmp_g_counter = 0;

        char flagsBuf[BUFFER_SIZE_FLAGS] = {};
        flagsToCptr(Flags(getFlagsRegister(ctx).value), flagsBuf);

        if (destRegister || destMemoryAddress) {
            auto& sb = ctx.__verbosecity_buff; sb.clear();
            detail::encodeBasicInstruction(sb, inst, ctx.decodingOpts);
            const char* encodedInst = sb.view().buff;

            writeDirectBold("(%lld) %s", ++tmp_g_counter, encodedInst);

            if (destRegister) {
                constexpr const char* fmtCptr = " ; %s:  0x%X-> 0x%X, ip:  0x%X-> 0x%X, flags: %s";
                const char* rtype = regTypeToCptr(destRegister->type);
                writeLine(fmtCptr, rtype, old, destRegister->value, ip.value, nextIp, flagsBuf);
            }
            else if (destMemoryAddress) {
                constexpr const char* fmtCptr = " ; [0x%06X]:  0x%X-> 0x%X, ip:  0x%X-> 0x%X, flags: %s";
                addr_off targetAddrOff = addr_off(reinterpret_cast<u8*>(destMemoryAddress) - ctx.memory);
                writeLine(fmtCptr, targetAddrOff, old, *destMemoryAddress, ip.value, nextIp, flagsBuf);
            }
        }
        else {
            writeLineBold("(%lld) %s", ++tmp_g_counter, instTypeToCptr(inst.type));
            writeLine(" -> ip: 0x%X->0x%X, flags: %s", ip.value, nextIp, flagsBuf);
            writeLine("");
        }
    }

    ip.value = nextIp;
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
    inst = ctx.instructions[addr_size(idx)];
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
        writeLine("%s\n", info);
#endif
        emulateNext(ctx, inst);
    }
}

} // namespace asm8086
