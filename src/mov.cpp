#include "mov.h"
#include "utils.h"

void MovInst_v3::encode(core::str_builder<>& sb) const {
    sb.append("mov ");
    sb.append(regToCptr(reg, w));
    sb.append(", ");
    appendIntToSb_AsImmediate(sb, data);
}

void MovInst_v4::encode(core::str_builder<>& sb) const {
    sb.append("mov ");
    if (d) {
        sb.append("[");
        appendIntToSb_AsImmediate(sb, addr);
        sb.append("]");
        sb.append(", ");
        sb.append("ax");
    }
    else {
        sb.append("ax");
        sb.append(", ");
        sb.append("[");
        appendIntToSb_AsImmediate(sb, addr);
        sb.append("]");
    }
}
