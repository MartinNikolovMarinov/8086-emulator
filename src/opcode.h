#pragma once

#include "init_core.h"

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

namespace asm8086 {

// Enumeration for all instruction opcodes. From Table 4-12. 8086/8088 Instruction Set.
enum Opcode : u8 {
    // 4 bit opcodes
    MOV_IMM_TO_REG                      = 0b1011, // mov 3

    // 6 bit opcodes
    MOV_REG_OR_MEM_TO_OR_FROM_REG       = 0b100010, // mov 1
    ADD_REG_OR_MEM_WITH_REG_TO_EDIT     = 0b000000, // add 1
    IMM_TO_FROM_REG_OR_MEM              = 0b100000, // add/sub/cmp 2 (this is a very special boy...)
    SUB_REG_OR_MEM_WITH_REG_TO_EDIT     = 0b001010, // sub 1
    CMP_REG_OR_MEM_WITH_REG             = 0b001110, // cmp 1

    // 7 bit opcodes
    MOV_IMM_TO_REG_OR_MEM               = 0b1100011, // mov 2
    MOV_MEM_TO_ACC                      = 0b1010000, // mov 4
    MOV_ACC_TO_MEM                      = 0b1010001, // mov 5
    ADD_IMM_TO_ACC                      = 0b0000010, // add 3
    SUB_IMM_FROM_ACC                    = 0b0010110, // sub 2
    CMP_IMM_WITH_ACC                    = 0b0011110, // cmp 3

    // 8 bit opcodes
    MOV_REG_OR_MEMORY_TO_SEGMENT_REG    = 0b10001110, // 8 bits
    MOV_SEGMENT_REG_TO_REG_OR_MEMORY    = 0b10001100, // 8 bits
};

const char* opcodeToCptr(Opcode o);
Opcode opcodeDecode(u8 opcodeByte);

} // namespace asm8086
