#pragma once

#include "init_core.h"
#include "mod.h"

const char* regToCptr(u8 reg, u8 w);
const char* rmEffectiveAddrCalc(u8 rm);
void appendIntToSb_AsImmediate(core::str_builder<>& sb, u16 i);
void appendIntToSb_AsDisp(core::str_builder<>& sb, u16 i, Mod mod);
bool isSignbitSet(u8 i);
bool isSignbitSet(u16 i);
