#include "opcode.h"

const char* opcodeToCptr(Opcode o) {
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

