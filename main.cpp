#include <init_core.h>

#include <fmt/format.h>
#include <fcntl.h>

/**
 * IMPORTANT:
 * Synthesized information for machine instruction encoding and decoding, as described on page 160 in the Intel 8086 CPU
 * family manual.
 *
 * Machine instruction vary from 1 to 6 bytes in length.
 *
 * # Byte 1 is called the opcode byte.
 *
 * * First 5 bits are the instruction type (opcode)
 * * The next bit is the direction bit (d)
 *     * 1 - the REG field in the second byte identifies the destination operand (r/m to reg)
 *     * 0 - the REG field in the second byte identifies the source operand (reg to r/m)
 * * The next bit destinguishes between byte and word operations (w)
 *     * 1 - word operation
 *     * 0 - byte operation
 * * Instead of (d) and (w) there are 3 more single-bit fields, that take their place in some instructions, these are:
 *     * (s) - sign extension bit, which is used in conjunction with (w) to indicate sign extension.
 *     * (z) - zero extension bit, which destinguishes between zero and sign extension.
 *     * (v) - overflow bit, which is used as a compare bit with the zero flag in conditional repeat and loop
 *              instructions.
 *
 * # Byte 2 usually identifies the instruction's operands.
 *
 * * The first 2 bits are the mod field (mod). It indicates whether one of the operands is in memory or whether both
 *   operands are registers.
 *      * 00 - memory mode, no displacement follows.
 *      * 01 - memory mode, 8-bit displacement follows.
 *      * 10 - memory mode, 16-bit displacement follows.
 *      * 11 - register mode, no displacement follows.
 * * The next 3 bits are the register field (reg). It identifies a register that is one of the instruction operands.
 *     * 000 - w == 0: AL, w == 1: AX
 *     * 001 - w == 0: CL, w == 1: CX
 *     * 010 - w == 0: DL, w == 1: DX
 *     * 011 - w == 0: BL, w == 1: BX
 *     * 100 - w == 0: AH, w == 1: SP
 *     * 101 - w == 0: CH, w == 1: BP
 *     * 110 - w == 0: DH, w == 1: SI
 *     * 111 - w == 0: BH, w == 1: DI
 * * The last 3 bits are the r/m field (rm). It depends on how the (mod) field is set. If (mod) is 11
 *   (register-to-register mode), then (rm) identifies the second register operand. If (mod) selects memory mode, then
 *   (rm) indicates how the effective address of the memory operand is to be calculated.
 *     * mod == 11 - REGISTER TO REGISTER MODE, NO DISPLACEMENT
 *         * 000 (rm) - w == 0: AL, w == 1: AX
 *         * 001 (rm) - w == 0: CL, w == 1: CX
 *         * 010 (rm) - w == 0: DL, w == 1: DX
 *         * 011 (rm) - w == 0: BL, w == 1: BX
 *         * 100 (rm) - w == 0: AH, w == 1: SP
 *         * 101 (rm) - w == 0: CH, w == 1: BP
 *         * 110 (rm) - w == 0: DH, w == 1: SI
 *         * 111 (rm) - w == 0: BH, w == 1: DI
 *     * mod == 00 - MEMORY MODE, NO DISPLACEMENT
 *         * 000 (rm) - BX + SI
 *         * 001 (rm) - BX + DI
 *         * 010 (rm) - BP + SI
 *         * 011 (rm) - BP + DI
 *         * 100 (rm) - SI
 *         * 101 (rm) - DI
 *         * 110 (rm) - Direct address
 *         * 111 (rm) - BX
 *     * mod == 01 - MEMORY MODE, 8-BIT DISPLACEMENT
 *         * 000 (rm) - BX + SI + disp8
 *         * 001 (rm) - BX + DI + disp8
 *         * 010 (rm) - BP + SI + disp8
 *         * 011 (rm) - BP + DI + disp8
 *         * 100 (rm) - SI + disp8
 *         * 101 (rm) - DI + disp8
 *         * 110 (rm) - BP + disp8
 *         * 111 (rm) - BX + disp8
 *     * mod == 10 - MEMORY MODE, 16-BIT DISPLACEMENT
 *         * 000 (rm) - BX + SI + disp16
 *         * 001 (rm) - BX + DI + disp16
 *         * 010 (rm) - BP + SI + disp16
 *         * 011 (rm) - BP + DI + disp16
 *         * 100 (rm) - SI + disp16
 *         * 101 (rm) - DI + disp16
 *         * 110 (rm) - BP + disp16
 *         * 111 (rm) - BX + disp16
 *
 * # Bytes 3 through 6 of an instruction are optional fields that usually contain the displacement value of memory
 *   operand and/or the actual value of an immediate constant operand.
*/

// Enumeration for all instruction opcodes. From Table 4-12. 8086/8088 Instruction Set.
enum Opcode : u8 {
    // 4 bit opcodes
    MOV_IMM_TO_REG                      = 0b1011,

    // 6 bit opcodes
    MOV_REG_OR_MEM_TO_OR_FROM_REG       = 0b100010,
    MOV_MEM_TO_ACC                      = 0b101000,
    MOV_ACC_TO_MEM                      = 0b101001,

    // 7 bit opcodes
    MOV_IMM_TO_REG_OR_MEM               = 0b1100011,

    // 8 bit opcodes
    MOV_REG_OR_MEMORY_TO_SEGMENT_REG    = 0b10001110, // 8 bits
    MOV_SEGMENT_REG_TO_REG_OR_MEMORY    = 0b10001100, // 8 bits
};

static constexpr const char* opcodeToCptr(Opcode o) {
    switch (o) {
        case MOV_IMM_TO_REG:                     return "MOV immediate to register";
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:      return "MOV register/memory to/from register";
        case MOV_MEM_TO_ACC:                     return "MOV memory to accumulator";
        case MOV_ACC_TO_MEM:                     return "MOV accumulator to memory";
        case MOV_IMM_TO_REG_OR_MEM:              return "MOV immediate to register/memory";
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:   return "MOV register/memory to segment register";
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:   return "MOV segment register to register/memory";
    }

    return "UNKNOWN OPCODE";
}

static constexpr Opcode opcodeDecode(u8 opcodeByte) {
    // Check 8 bit opcodes:
    switch (opcodeByte) {
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG: return MOV_REG_OR_MEMORY_TO_SEGMENT_REG;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY: return MOV_SEGMENT_REG_TO_REG_OR_MEMORY;
    }

    // Check 7 bit opcodes:
    opcodeByte = opcodeByte >> 1;
    switch (opcodeByte) {
        case MOV_IMM_TO_REG_OR_MEM: return MOV_IMM_TO_REG_OR_MEM;
    }

    // Check 6 bit opcodes:
    opcodeByte = opcodeByte >> 1;
    switch (opcodeByte) {
        case MOV_REG_OR_MEM_TO_OR_FROM_REG: return MOV_REG_OR_MEM_TO_OR_FROM_REG;
        case MOV_MEM_TO_ACC:                return MOV_MEM_TO_ACC;
        case MOV_ACC_TO_MEM:                return MOV_ACC_TO_MEM;
    }

    // Check 5 bit opcodes:
    opcodeByte = opcodeByte >> 1;
    // TODO: Don't forget to add a switch for 5 bit opcodes, when support is added for any of them.

    // Check 4 bit opcodes:
    opcodeByte = opcodeByte >> 1;
    switch (opcodeByte) {
        case MOV_IMM_TO_REG: return MOV_IMM_TO_REG;
    }

    Panic(false, "Opcode unsupported or invalid");
    return Opcode(0);
}

static constexpr i32 MAX_8086_INST_SIZE = 6;

enum Mod : u8 {
    MOD_MEMORY_NO_DISPLACEMENT               = 0b00,
    MOD_MEMORY_8_BIT_DISPLACEMENT            = 0b01,
    MOD_MEMORY_16_BIT_DISPLACEMENT           = 0b10,
    MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT = 0b11,
};

struct MovInst_v1 {
    u8 d : 1;
    u8 w : 1;
    u8 reg : 3;
    u8 rm : 3;
    Mod mod : 2;
    u16 disp : 16;

    void encode(core::str_builder<>& sb) const {
        sb.append("mov ");

        if (mod == MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT) {
            if (d) {
                encodeReg(sb, reg);
                sb.append(", ");
                encodeReg(sb, rm);
            }
            else {
                encodeReg(sb, rm);
                sb.append(", ");
                encodeReg(sb, reg);
            }
        }
        else {
            Assert(false, "Not implemented");
        }
    }

private:

    void encodeReg(core::str_builder<>& sb, u8 reg) const {
        if (w) {
            switch(reg) {
                case 0b000: sb.append("ax"); break;
                case 0b001: sb.append("cx"); break;
                case 0b010: sb.append("dx"); break;
                case 0b011: sb.append("bx"); break;
                case 0b100: sb.append("sp"); break;
                case 0b101: sb.append("bp"); break;
                case 0b110: sb.append("si"); break;
                case 0b111: sb.append("di"); break;
                default: Panic(false, "Invalid register");
            }
        }
        else {
            switch(reg) {
                case 0b000: sb.append("al"); break;
                case 0b001: sb.append("cl"); break;
                case 0b010: sb.append("dl"); break;
                case 0b011: sb.append("bl"); break;
                case 0b100: sb.append("ah"); break;
                case 0b101: sb.append("ch"); break;
                case 0b110: sb.append("dh"); break;
                case 0b111: sb.append("bh"); break;
                default: Panic(false, "Invalid register");
            }
        }
    }
};

struct MovInst_v3 {
    u8 w : 1;
    u8 reg : 3;
    i16 data : 16; // NOTE: I can't determine if data should be signed or unsigned literal.

    void encode(core::str_builder<>& sb) const {
        sb.append("mov ");
        encodeReg(sb, reg);
        sb.append(", ");
        char dataToStr[15] = {};
        core::int_to_cptr(data, dataToStr);
        sb.append(dataToStr);
    }

private:

    void encodeReg(core::str_builder<>& sb, u8 reg) const {
        if (w) {
            switch(reg) {
                case 0b000: sb.append("ax"); break;
                case 0b001: sb.append("cx"); break;
                case 0b010: sb.append("dx"); break;
                case 0b011: sb.append("bx"); break;
                case 0b100: sb.append("sp"); break;
                case 0b101: sb.append("bp"); break;
                case 0b110: sb.append("si"); break;
                case 0b111: sb.append("di"); break;
                default: Panic(false, "Invalid register");
            }
        }
        else {
            switch(reg) {
                case 0b000: sb.append("al"); break;
                case 0b001: sb.append("cl"); break;
                case 0b010: sb.append("dl"); break;
                case 0b011: sb.append("bl"); break;
                case 0b100: sb.append("ah"); break;
                case 0b101: sb.append("ch"); break;
                case 0b110: sb.append("dh"); break;
                case 0b111: sb.append("bh"); break;
                default: Panic(false, "Invalid register");
            }
        }
    }
};

struct Inst8086 {
    Opcode opcode;
    union {
        MovInst_v1 movRegOrMemToOrFromReg;
        MovInst_v3 movImmToReg;
    };

    void encode(core::str_builder<>& sb) const {
        switch (opcode) {
            case MOV_REG_OR_MEM_TO_OR_FROM_REG: movRegOrMemToOrFromReg.encode(sb); break;
            case MOV_IMM_TO_REG:                movImmToReg.encode(sb);            break;
            default:                            Panic(false, "Opcode unsupported or invalid");
        }
    }
};

Inst8086 decodeInst(core::arr<u8>& bytes, i32& idx) {
    auto o   = [](u8 b) -> Opcode { return opcodeDecode(b); };
    auto w   = [](u8 b) -> bool   { return (b >> 0) & 0b1; };
    auto d   = [](u8 b) -> bool   { return (b >> 1) & 0b1; };
    auto mod = [](u8 b) -> Mod    { return Mod((b >> 6) & 0b11); };
    auto reg = [](u8 b) -> u8     { return (b >> 3) & 0b111; };
    auto rm  = [](u8 b) -> u8     { return (b >> 0) & 0b111; };

    u8 byte1 = bytes[idx++];
    Inst8086 res = {};
    res.opcode = o(byte1);

    switch (res.opcode) {
        case MOV_IMM_TO_REG:
        {
            MovInst_v3 inst = {};
            inst.w = (byte1 >> 3) & 0b1; // For this command w is in the first byte!
            inst.reg = reg(byte1);

            if (inst.w == 0) {
                u8 byte2 = bytes[idx++];
                inst.data = i16(i8(byte2)); // Be very careful not to drop the sign!
            }
            else {
                u8 byte2 = bytes[idx++];
                u8 byte3 = bytes[idx++];
                inst.data = i16(byte3 << 8) | i16(byte2);
            }

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
            inst.rm = rm(byte2);
            inst.reg = reg(byte2);

            if (inst.mod == MOD_MEMORY_8_BIT_DISPLACEMENT) {
                u8 byte3 = bytes[idx++];
                inst.disp = u16(byte3);
            }
            else if (inst.mod == MOD_MEMORY_16_BIT_DISPLACEMENT) {
                u8 byte3 = bytes[idx++];
                u8 byte4 = bytes[idx++];
                inst.disp = u16(byte4 << 8) | u16(byte3);
            }

            res.movRegOrMemToOrFromReg = inst;
            return res;
        }
        case MOV_MEM_TO_ACC:
            Assert(false, "Not implemented");
            return res;
        case MOV_ACC_TO_MEM:
            Assert(false, "Not implemented");
            return res;
        case MOV_IMM_TO_REG_OR_MEM:
            Assert(false, "Not implemented");
            return res;
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:
            Assert(false, "Not implemented");
            return res;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:
            Assert(false, "Not implemented");
            return res;
    }

    return res;
}

i32 main(i32 argc, char const** argv) {
    initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666));
    core::str_builder encodedStream;
    encodedStream.append("bits 16\n\n"); // TODO: Hardcoding 16 bit mode, but this should be detected from the binary.
    i32 idx = 0;
    i32 instCount = 0;
    while (idx < binaryData.len()) {
        auto inst = decodeInst(binaryData, idx);
        inst.encode(encodedStream);
        encodedStream.append("\n");
        instCount++;
    }

    fmt::print("{}\n", encodedStream.view().buff);
    fmt::print("Decoded {} instructions\n", instCount);

    return 0;
}
