#pragma once

#include "init_core.h"

const char* regToCptr(u8 reg, u8 w);
const char* rmEffectiveAddrCalc(u8 rm);
void appendIntToSbAsImmediate(core::str_builder<>& sb, u16 i);
void appendIntToSbAsCalculation(core::str_builder<>& sb, u16 i);
bool isSignbitSet(u16 i);
