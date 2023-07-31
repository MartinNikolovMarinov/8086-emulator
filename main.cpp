#include <init_core.h>

#include <fmt/format.h>
#include <fcntl.h>

// enum Opcodes {
//     MOV_REG_TO_REG = 0b100010,
//     SENTINEL
// };

// // NOTE: Machine instruction encoding and decoding is described on page 160 in the manual.

// struct MovInst {
//     u8 byte1;
//     u8 byte2;

//     u8 opcode() const { return byte1 >> 2; }
//     u8 d() const { return (byte1 >> 1) & 0b1; }
//     u8 w() const { return byte1 & 0b1; }
//     u8 mod() const { return byte2 >> 6; }
//     u8 reg() const { return (byte2 >> 3) & 0b111; }
//     u8 rm() const { return byte2 & 0b111; }
// };

// bool decodeRegs(u32 r, u32 w, std::string& out) {
//     switch (r) {
//         case 0b000: out += (w == 0) ? "al" : "ax"; break;
//         case 0b001: out += (w == 0) ? "cl" : "cx"; break;
//         case 0b010: out += (w == 0) ? "dl" : "dx"; break;
//         case 0b011: out += (w == 0) ? "bl" : "bx"; break;
//         case 0b100: out += (w == 0) ? "ah" : "sp"; break;
//         case 0b101: out += (w == 0) ? "ch" : "bp"; break;
//         case 0b110: out += (w == 0) ? "dh" : "si"; break;
//         case 0b111: out += (w == 0) ? "bh" : "di"; break;
//         default: return false;
//     }
//     return true;
// }

// bool decode(MovInst& inst, std::string& out) {
//     out += "mov ";
//     if (bool ok = decodeRegs(inst.rm(), inst.w(), out); !ok) return false;
//     out += ", ";
//     if (inst.mod() == 0b11) {
//         if (bool ok = decodeRegs(inst.reg(), inst.w(), out); !ok) return false;
//     } else {
//         Assert(false, "not implemented yet");
//     }
//     out += "\n";
//     return true;
// }

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
 *     * mod == 11 - REGISTER TO REGISTER MODE
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

template <i32 TByteSize>
struct InstASM86 {
    static_assert(TByteSize >= 1 && TByteSize <= 6, "InstASM86 size must be between 1 and 6 bytes");

    u8 bytes[TByteSize] = {};
};

i32 main(i32 argc, char const** argv) {
    initCore(argc, argv);

    auto binaryData = ValueOrDie(core::file_read_full(g_cmdLineArgs.fileName, O_RDONLY, 0666));
    std::string out = "bits 16\n";
    for (i32 i = 0; i < binaryData.len(); i+=2) {
        MovInst mov;
        mov.byte1 = binaryData[i];
        mov.byte2 = binaryData[i+1];
        Assert(decode(mov, out));
    }

    fmt::print("{}\n", out.data());
    return 0;
}
