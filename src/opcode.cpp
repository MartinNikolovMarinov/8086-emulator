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
        case ADD_IMM_TO_ACC:                     return "Add Immediate to accumulator";

        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT:    return "SUB register/memory with register to edit";
        case SUB_IMM_FROM_ACC:                   return "SUB Immediate from accumulator";

        case CMP_REG_OR_MEM_WITH_REG:            return "CMP register/memory with register";
        case CMP_IMM_WITH_ACC:                   return "CMP immediate with accumulator";

        case JNEZ_ON_NOT_EQ_NOR_ZERO:            return "JNEZ jump on not equal/not zero";
    }

    return "UNKNOWN OPCODE";
}

Opcode opcodeDecode(u8 opcodeByte) {
    // Check 8 bit opcodes:
    switch (opcodeByte) {
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG: return MOV_REG_OR_MEMORY_TO_SEGMENT_REG;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY: return MOV_SEGMENT_REG_TO_REG_OR_MEMORY;
        case JNEZ_ON_NOT_EQ_NOR_ZERO:          return JNEZ_ON_NOT_EQ_NOR_ZERO;
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

namespace {

static FieldDisplacements displacementsLT[core::MAX_U8];
static core::once initDisplacementsLTOnce;

constexpr FieldDisplacements::Displacement DEFAULT_NOT_SET  = { 0, 0, -1 };
constexpr FieldDisplacements::Displacement DEFAULT_OPTIONAL = { 0, 0b11111111, 2 };
constexpr FieldDisplacements::Displacement DEFAULT_W        = { 0, 0b00000001, 0 };
constexpr FieldDisplacements::Displacement DEFAULT_D        = { 1, 0b00000010, 0 };
constexpr FieldDisplacements::Displacement DEFAULT_S        = { 1, 0b00000010, 0 };
constexpr FieldDisplacements::Displacement DEFAULT_MOD      = { 6, 0b11000000, 1 };
constexpr FieldDisplacements::Displacement DEFAULT_REG      = { 3, 0b00111000, 1 };
constexpr FieldDisplacements::Displacement DEFAULT_RM       = { 0, 0b00000111, 1 };

constexpr i8 NO_FIXED_SIZE = -1;
constexpr i8 FIXED_SIZE_BYTE = 0;
constexpr i8 FIXED_SIZE_WORD = 1;

void initDisplacementsLT() {
    displacementsLT[MOV_REG_OR_MEM_TO_OR_FROM_REG] = {
        { 2, 0b11111100, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_MOD, DEFAULT_REG, DEFAULT_RM,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        NO_FIXED_SIZE
    };
    displacementsLT[MOV_IMM_TO_REG_OR_MEM] = {
        { 1, 0b11111110, 0 }, DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_MOD, DEFAULT_REG, DEFAULT_RM,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        NO_FIXED_SIZE
    };
    displacementsLT[MOV_IMM_TO_REG] = {
        { 4, 0b11110000, 0 }, DEFAULT_NOT_SET, DEFAULT_NOT_SET, { 3, 0b00001000, 0 },
        DEFAULT_NOT_SET, { 0, 0b00000111, 0 }, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        NO_FIXED_SIZE
    };
    displacementsLT[MOV_MEM_TO_ACC] = {
        { 1, 0b11111110, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        NO_FIXED_SIZE
    };
    displacementsLT[MOV_ACC_TO_MEM] = {
        { 1, 0b11111110, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        NO_FIXED_SIZE
    };
    // displacementsLT[MOV_REG_OR_MEMORY_TO_SEGMENT_REG] = {};
    // displacementsLT[MOV_SEGMENT_REG_TO_REG_OR_MEMORY] = {};

    displacementsLT[ADD_REG_OR_MEM_WITH_REG_TO_EDIT] = {
        { 2, 0b11111100, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_MOD, DEFAULT_REG, DEFAULT_RM,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        NO_FIXED_SIZE
    };
    displacementsLT[IMM_TO_FROM_REG_OR_MEM] = {
        { 2, 0b11111100, 0 }, DEFAULT_NOT_SET, DEFAULT_S, DEFAULT_W,
        DEFAULT_MOD, DEFAULT_REG, DEFAULT_RM,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        NO_FIXED_SIZE
    };
    displacementsLT[ADD_IMM_TO_ACC] = {
        { 1, 0b11111110, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        NO_FIXED_SIZE
    };

    displacementsLT[SUB_REG_OR_MEM_WITH_REG_TO_EDIT] = {
        { 2, 0b11111100, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_MOD, DEFAULT_REG, DEFAULT_RM,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        NO_FIXED_SIZE
    };
    displacementsLT[SUB_IMM_FROM_ACC] = {
        { 1, 0b11111110, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        NO_FIXED_SIZE
    };

    displacementsLT[CMP_REG_OR_MEM_WITH_REG] = {
        { 2, 0b11111100, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_MOD, DEFAULT_REG, DEFAULT_RM,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        NO_FIXED_SIZE
    };
    displacementsLT[CMP_IMM_WITH_ACC] = {
        { 1, 0b11111110, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        NO_FIXED_SIZE
    };

    displacementsLT[JNEZ_ON_NOT_EQ_NOR_ZERO] = {
        { 0, 0b11111111, 0 }, DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_OPTIONAL, DEFAULT_NOT_SET,
        FIXED_SIZE_BYTE
    };
}

} // namespace

FieldDisplacements getFieldDisplacements(Opcode opcode) {
    initDisplacementsLTOnce.do_once(initDisplacementsLT);
    FieldDisplacements fd = displacementsLT[opcode];
    return fd;
}

} // namespace asm8086
