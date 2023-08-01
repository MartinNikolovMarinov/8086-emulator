#include "utils.h"

const char* regToCptr(u8 reg, u8 w) {
    if (w) {
        switch(reg) {
            case 0b000: return "ax";
            case 0b001: return "cx";
            case 0b010: return "dx";
            case 0b011: return "bx";
            case 0b100: return "sp";
            case 0b101: return "bp";
            case 0b110: return "si";
            case 0b111: return "di";
        }
    }
    else {
        switch(reg) {
            case 0b000: return "al";
            case 0b001: return "cl";
            case 0b010: return "dl";
            case 0b011: return "bl";
            case 0b100: return "ah";
            case 0b101: return "ch";
            case 0b110: return "dh";
            case 0b111: return "bh";
        }
    }

    Panic(false, "Invalid register");
    return "INVALID REGISTER";
}

const char* rmEffectiveAddrCalc(u8 rm) {
    switch (rm) {
        case 0b000: return "bx + si";
        case 0b001: return "bx + di";
        case 0b010: return "bp + si";
        case 0b011: return "bp + di";
        case 0b100: return "si";
        case 0b101: return "di";
        case 0b110: return "bp";
        case 0b111: return "bx";
    }

    Panic(false, "Invalid register");
    return "INVALID REGISTER";
}

namespace {

template <typename TInt>
void appendIntToSbAsImmediate(core::str_builder<>& sb, TInt i) {
    char ncptr[8] = {};
    core::int_to_cptr(i, ncptr);
    sb.append(ncptr);
}

} // namespace

void appendIntToSbAsImmediate(core::str_builder<>& sb, u16 i) {
    if (isSignbitSet(i)) {
        appendIntToSbAsImmediate<i16>(sb, i16(i));
    }
    else {
        appendIntToSbAsImmediate<u16>(sb, i);
    }
}

void appendIntToSbAsCalculation(core::str_builder<>& sb, u16 i) {
    if (isSignbitSet(i)) {
        sb.append(" - ");
        i16 iabs = core::abs(i16(i));
        appendIntToSbAsImmediate<i16>(sb, iabs);
    }
    else {
        sb.append(" + ");
        appendIntToSbAsImmediate<i16>(sb, i16(i));
    }
}

bool isSignbitSet(u16 i) { return (i & 0x8000) != 0; }
