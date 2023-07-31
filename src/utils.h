#pragma once

#include "init_core.h"

const char* regToCptr(u8 reg, u8 w);
const char* rmEffectiveAddrCalc(u8 rm);
void appendIntToSb(core::str_builder<>& sb, i16 i);
