#include <opcodes.h>

namespace asm8086 {

const char* opcodeToCptr(Opcode o) {
    switch (o) {
        case MOV_IMM_TO_REG:                       return "MOV immediate to register";
        case MOV_REG_OR_MEM_TO_OR_FROM_REG:        return "MOV register/memory to/from register";
        case MOV_MEM_TO_ACC:                       return "MOV memory to accumulator";
        case MOV_ACC_TO_MEM:                       return "MOV accumulator to memory";
        case MOV_IMM_TO_REG_OR_MEM:                return "MOV immediate to register/memory";
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:     return "MOV register/memory to segment register";
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:     return "MOV segment register to register/memory";

        case ADD_REG_OR_MEM_WITH_REG_TO_EDIT:      return "Add register/memory with register to edit";
        case IMM_TO_FROM_REG_OR_MEM:               return "Immediate to register/memory";
        case ADD_IMM_TO_ACC:                       return "Add Immediate to accumulator";

        case SUB_REG_OR_MEM_WITH_REG_TO_EDIT:      return "SUB register/memory with register to edit";
        case SUB_IMM_FROM_ACC:                     return "SUB Immediate from accumulator";

        case CMP_REG_OR_MEM_WITH_REG:              return "CMP register/memory with register";
        case CMP_IMM_WITH_ACC:                     return "CMP immediate with accumulator";

        case JE_JZ_ON_EQ_ZERO:                     return "JE/JZ Jump on equal/zero";
        case JL_JNGE_ON_LESS_NOT_GE_OR_EQ:         return "JL/JNGE Jump on less/not greater or equal";
        case JLE_JNG_ON_LESS_OR_EQ_NOT_GE:         return "JLE/JNG Jump on less or equal/not greater";
        case JB_JNAE_ON_BELOW_NOT_ABOVE_OR_EQ:     return "JB/JNAE Jump on below/not above or equal";
        case JBE_JNA_ON_BELOW_OR_EQ_NOT_ABOVE:     return "JBE/JNA Jump on below or equal/not above";
        case JP_JPE_ON_PARITY_EVEN:                return "JP/JPE Jump on parity/parity even";
        case JO_ON_OVERFLOW:                       return "JO Jump on overflow";
        case JS_ON_SIGN:                           return "JS Jump on sign";
        case JNE_JNZ_ON_NOT_EQ_NOR_ZERO:           return "JNEZ jump on not equal/not zero";
        case JNL_JGE_ON_NOT_LESS_GE_OR_EQ:         return "JNL/JGE Jump on not less/greater or equal";
        case JNLE_JG_ON_NOT_LESS_OR_EQ_GE:         return "JNLE/JG Jump on not less or equal/greater";
        case JNB_JAE_ON_NOT_BELOW_ABOVE_OR_EQ:     return "JNB/JAE Jump on not below/above or equal";
        case JNBE_JA_ON_NOT_BELOW_OR_EQ_ABOVE:     return "JNBE/JA Jump on not below or equal/above";
        case JNP_JPO_ON_NOT_PAR_PAR_ODD:           return "JNP/JPO Jump on not parity/parity odd";
        case JNO_ON_NOT_OVERFLOW:                  return "JNO Jump on not overflow";
        case JNS_ON_NOT_SIGN:                      return "Jump on not sign";
        case LOOP_CX_TIMES:                        return "Loop CX times";
        case LOOPZ_LOOPE_WHILE_ZERO_EQ:            return "Loop while zero/equal";
        case LOOPNZ_LOOPNE_WHILE_NOT_ZERO_EQ:      return "Loop while not zero/equal";
        case JCXZ_ON_CX_ZERO:                      return "Jump on CX zero";
    }

    return "UNKNOWN OPCODE";
}

Opcode opcodeDecode(u8 opcodeByte) {
    // Check 8 bit opcodes:
    switch (opcodeByte) {
        case MOV_REG_OR_MEMORY_TO_SEGMENT_REG:     return MOV_REG_OR_MEMORY_TO_SEGMENT_REG;
        case MOV_SEGMENT_REG_TO_REG_OR_MEMORY:     return MOV_SEGMENT_REG_TO_REG_OR_MEMORY;
        case JE_JZ_ON_EQ_ZERO:                     return JE_JZ_ON_EQ_ZERO;
        case JL_JNGE_ON_LESS_NOT_GE_OR_EQ:         return JL_JNGE_ON_LESS_NOT_GE_OR_EQ;
        case JLE_JNG_ON_LESS_OR_EQ_NOT_GE:         return JLE_JNG_ON_LESS_OR_EQ_NOT_GE;
        case JB_JNAE_ON_BELOW_NOT_ABOVE_OR_EQ:     return JB_JNAE_ON_BELOW_NOT_ABOVE_OR_EQ;
        case JBE_JNA_ON_BELOW_OR_EQ_NOT_ABOVE:     return JBE_JNA_ON_BELOW_OR_EQ_NOT_ABOVE;
        case JP_JPE_ON_PARITY_EVEN:                return JP_JPE_ON_PARITY_EVEN;
        case JO_ON_OVERFLOW:                       return JO_ON_OVERFLOW;
        case JS_ON_SIGN:                           return JS_ON_SIGN;
        case JNE_JNZ_ON_NOT_EQ_NOR_ZERO:           return JNE_JNZ_ON_NOT_EQ_NOR_ZERO;
        case JNL_JGE_ON_NOT_LESS_GE_OR_EQ:         return JNL_JGE_ON_NOT_LESS_GE_OR_EQ;
        case JNLE_JG_ON_NOT_LESS_OR_EQ_GE:         return JNLE_JG_ON_NOT_LESS_OR_EQ_GE;
        case JNB_JAE_ON_NOT_BELOW_ABOVE_OR_EQ:     return JNB_JAE_ON_NOT_BELOW_ABOVE_OR_EQ;
        case JNBE_JA_ON_NOT_BELOW_OR_EQ_ABOVE:     return JNBE_JA_ON_NOT_BELOW_OR_EQ_ABOVE;
        case JNP_JPO_ON_NOT_PAR_PAR_ODD:           return JNP_JPO_ON_NOT_PAR_PAR_ODD;
        case JNO_ON_NOT_OVERFLOW:                  return JNO_ON_NOT_OVERFLOW;
        case JNS_ON_NOT_SIGN:                      return JNS_ON_NOT_SIGN;
        case LOOP_CX_TIMES:                        return LOOP_CX_TIMES;
        case LOOPZ_LOOPE_WHILE_ZERO_EQ:            return LOOPZ_LOOPE_WHILE_ZERO_EQ;
        case LOOPNZ_LOOPNE_WHILE_NOT_ZERO_EQ:      return LOOPNZ_LOOPNE_WHILE_NOT_ZERO_EQ;
        case JCXZ_ON_CX_ZERO:                      return JCXZ_ON_CX_ZERO;
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

    // Check 4 bit opcodes:
    opcodeByte = opcodeByte >> 1;
    switch (opcodeByte) {
        case MOV_IMM_TO_REG: return MOV_IMM_TO_REG;
    }

    Panic(false, "Opcode unsupported or invalid");
    return Opcode(0);
}

namespace {

static bool isInitDisplacementsLTCalled = false;
static FieldDisplacements displacementsLT[core::MAX_U8];

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

constexpr FieldDisplacements DEFAULT_JMP = {
    { 0, 0b11111111, 0 }, DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
    DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
    DEFAULT_NOT_SET, DEFAULT_NOT_SET,
    DEFAULT_OPTIONAL, DEFAULT_NOT_SET,
    FIXED_SIZE_BYTE
};

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
        FIXED_SIZE_WORD
    };
    displacementsLT[MOV_ACC_TO_MEM] = {
        { 1, 0b11111110, 0 }, DEFAULT_D, DEFAULT_NOT_SET, DEFAULT_W,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        FIXED_SIZE_WORD
    };
    displacementsLT[MOV_REG_OR_MEMORY_TO_SEGMENT_REG] = {
        { 0, 0b11111111, 0 }, DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_MOD, DEFAULT_REG, DEFAULT_RM,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        NO_FIXED_SIZE
    };
    displacementsLT[MOV_SEGMENT_REG_TO_REG_OR_MEMORY] = {
        { 0, 0b11111111, 0 }, DEFAULT_NOT_SET, DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        DEFAULT_MOD, DEFAULT_REG, DEFAULT_RM,
        DEFAULT_OPTIONAL, DEFAULT_OPTIONAL,
        DEFAULT_NOT_SET, DEFAULT_NOT_SET,
        NO_FIXED_SIZE
    };

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

    displacementsLT[JE_JZ_ON_EQ_ZERO]                 = DEFAULT_JMP;
    displacementsLT[JL_JNGE_ON_LESS_NOT_GE_OR_EQ]     = DEFAULT_JMP;
    displacementsLT[JLE_JNG_ON_LESS_OR_EQ_NOT_GE]     = DEFAULT_JMP;
    displacementsLT[JB_JNAE_ON_BELOW_NOT_ABOVE_OR_EQ] = DEFAULT_JMP;
    displacementsLT[JBE_JNA_ON_BELOW_OR_EQ_NOT_ABOVE] = DEFAULT_JMP;
    displacementsLT[JP_JPE_ON_PARITY_EVEN]            = DEFAULT_JMP;
    displacementsLT[JO_ON_OVERFLOW]                   = DEFAULT_JMP;
    displacementsLT[JS_ON_SIGN]                       = DEFAULT_JMP;
    displacementsLT[JNE_JNZ_ON_NOT_EQ_NOR_ZERO]       = DEFAULT_JMP;
    displacementsLT[JNL_JGE_ON_NOT_LESS_GE_OR_EQ]     = DEFAULT_JMP;
    displacementsLT[JNLE_JG_ON_NOT_LESS_OR_EQ_GE]     = DEFAULT_JMP;
    displacementsLT[JNB_JAE_ON_NOT_BELOW_ABOVE_OR_EQ] = DEFAULT_JMP;
    displacementsLT[JNBE_JA_ON_NOT_BELOW_OR_EQ_ABOVE] = DEFAULT_JMP;
    displacementsLT[JNP_JPO_ON_NOT_PAR_PAR_ODD]       = DEFAULT_JMP;
    displacementsLT[JNO_ON_NOT_OVERFLOW]              = DEFAULT_JMP;
    displacementsLT[JNS_ON_NOT_SIGN]                  = DEFAULT_JMP;
    displacementsLT[LOOP_CX_TIMES]                    = DEFAULT_JMP;
    displacementsLT[LOOPZ_LOOPE_WHILE_ZERO_EQ]        = DEFAULT_JMP;
    displacementsLT[LOOPNZ_LOOPNE_WHILE_NOT_ZERO_EQ]  = DEFAULT_JMP;
    displacementsLT[JCXZ_ON_CX_ZERO]                  = DEFAULT_JMP;
}

} // namespace

FieldDisplacements getFieldDisplacements(Opcode opcode) {
    if (!isInitDisplacementsLTCalled) {
        initDisplacementsLT();
        isInitDisplacementsLTCalled = true;
    }
    FieldDisplacements fd = displacementsLT[opcode];
    return fd;
}

} // namespace asm8086
