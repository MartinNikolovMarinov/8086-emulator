#include "opcode.h"

namespace asm8086 {

const char* opcodeToCptr(Opcode o) {
    switch (o) {
        case MOV_IMM_TO_REG:                     return "MOV immediate to register";
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:      return "MOV register/memory to/from register";
        case MOV_MEM_TO_ACC:                     return "MOV memory to accumulator";
        case MOV_ACC_TO_MEM:                     return "MOV accumulator to memory";
        case MOV_IMM_TO_REG_OR_MEM:              return "MOV immediate to register/memory";
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:   return "MOV register/memory to segment register";
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:   return "MOV segment register to register/memory";

        case ADD_REG_OR_MEM_WITH_REG_TO_EDIT:    return "Add register/memory with register to edit";
        case IMM_TO_FROM_REG_OR_MEM:             return "Immediate to register/memory";
        case ADD_IMM_TO_ACC:                     return "Immediate to accumulator";

        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT:    return "Subtract register/memory with register to edit";
        case SUB_IMM_FROM_ACC:                   return "Immediate from accumulator";

        case CMP_REG_OR_MEM_WITH_REG:            return "Compare register/memory with register";
        case CMP_IMM_WITH_ACC:                   return "Compare immediate with accumulator";
    }

    return "UNKNOWN OPCODE";
}

Opcode opcodeDecode(u8 opcodeByte) {
    // Check 8 bit opcodes:
    switch (opcodeByte) {
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG: return MOV_REG_OR_MEMORY_TO_SEGMENT_REG;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY: return MOV_SEGMENT_REG_TO_REG_OR_MEMORY;
    }

    // Check 7 bit opcodes:
    opcodeByte = opcodeByte >> 1;
    switch (opcodeByte) {
        case MOV_IMM_TO_REG_OR_MEM: return MOV_IMM_TO_REG_OR_MEM;
        case MOV_MEM_TO_ACC:        return MOV_MEM_TO_ACC;
        case MOV_ACC_TO_MEM:        return MOV_ACC_TO_MEM;
        case ADD_IMM_TO_ACC:        return ADD_IMM_TO_ACC;
        case SUB_IMM_FROM_ACC:      return SUB_IMM_FROM_ACC;
        case CMP_IMM_WITH_ACC:      return CMP_IMM_WITH_ACC;
    }

    // Check 6 bit opcodes:
    opcodeByte = opcodeByte >> 1;
    switch (opcodeByte) {
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:   return MOV_REG_OR_MEM_TO_OR_FROM_REG;
        case ADD_REG_OR_MEM_WITH_REG_TO_EDIT: return ADD_REG_OR_MEM_WITH_REG_TO_EDIT;
        case IMM_TO_FROM_REG_OR_MEM:          return IMM_TO_FROM_REG_OR_MEM;
        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT: return SUB_REG_OR_MEM_WITH_REG_TO_EDIT;
        case CMP_REG_OR_MEM_WITH_REG:         return CMP_REG_OR_MEM_WITH_REG;
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

} // namespace asm8086
