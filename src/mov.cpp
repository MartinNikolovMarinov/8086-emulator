#include "mov.h"
#include "utils.h"

void MovInst_v1::encode(core::str_builder<>& sb) const {
    sb.append("mov ");

    if (mod == MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT) {
        if (d) {
            sb.append(regToCptr(reg, w));
            sb.append(", ");
            sb.append(regToCptr(rm, w));
        }
        else {
            sb.append(regToCptr(rm, w));
            sb.append(", ");
            sb.append(regToCptr(reg, w));
        }
    }
    else if (mod == MOD_MEMORY_NO_DISPLACEMENT) {
        bool isDirectAddressing = (rm == 0b110);

        if (d) {
            sb.append(regToCptr(reg, w));
            sb.append(", ");
            sb.append("[");
            if (isDirectAddressing) {
                appendIntToSb(sb, disp);
            } else {
                sb.append(rmEffectiveAddrCalc(rm));
            }
            sb.append("]");
        }
        else {
            sb.append("[");
            if (isDirectAddressing) {
                appendIntToSb(sb, disp);
            } else {
                sb.append(rmEffectiveAddrCalc(rm));
            }
            sb.append("]");
            sb.append(", ");
            sb.append(regToCptr(reg, w));
        }
    }
    else {
        // 8 or 16 bit displacement.
        if (d) {
            sb.append(regToCptr(reg, w));
            sb.append(", ");
            sb.append("[");
            sb.append(rmEffectiveAddrCalc(rm));
            if (disp != 0) {
                sb.append(disp > 0 ? " + " : " - ");
                appendIntToSb(sb, core::abs_slow(disp));
            }
            sb.append("]");
        }
        else {
            sb.append("[");
            sb.append(rmEffectiveAddrCalc(rm));
            if (disp != 0) {
                sb.append(disp > 0 ? " + " : " - ");
                appendIntToSb(sb, core::abs_slow(disp));
            }
            sb.append("]");
            sb.append(", ");
            sb.append(regToCptr(reg, w));
        }
    }
}

void MovInst_v2::encode(core::str_builder<>& sb) const {
    sb.append("mov ");

    if (mod == MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT) {
        sb.append(regToCptr(rm, w));
        sb.append(", ");
        sb.append(w ? "word" : "byte");
        sb.append(" ");
        appendIntToSb(sb, data);
    }
    else if (mod == MOD_MEMORY_NO_DISPLACEMENT) {
        sb.append("[");
        sb.append(rmEffectiveAddrCalc(rm));
        sb.append("]");
        sb.append(", ");
        sb.append(w ? "word" : "byte");
        sb.append(" ");
        appendIntToSb(sb, data);
    }
    else {
        // 8 or 16 bit displacement.
        sb.append("[");
        sb.append(rmEffectiveAddrCalc(rm));
        if (disp != 0) {
            sb.append(disp > 0 ? " + " : " - ");
            appendIntToSb(sb, core::abs_slow(disp));
        }
        sb.append("]");
        sb.append(", ");
        sb.append(w ? "word" : "byte");
        sb.append(" ");
        appendIntToSb(sb, data);
    }
}

void MovInst_v3::encode(core::str_builder<>& sb) const {
    sb.append("mov ");
    sb.append(regToCptr(reg, w));
    sb.append(", ");
    appendIntToSb(sb, data);
}

void MovInst_v4::encode(core::str_builder<>& sb) const {
    sb.append("mov ");
    if (d) {
        sb.append("[");
        appendIntToSb(sb, addr);
        sb.append("]");
        sb.append(", ");
        sb.append("ax");
    }
    else {
        sb.append("ax");
        sb.append(", ");
        sb.append("[");
        appendIntToSb(sb, addr);
        sb.append("]");
    }
}
