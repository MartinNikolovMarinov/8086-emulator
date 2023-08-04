#pragma once

#include "init_core.h"

namespace asm8086 {

inline constexpr bool isSignbitSet(u8 i)  { return (i & 0x80) != 0; }
inline constexpr bool isSignbitSet(u16 i) { return (i & 0x8000) != 0; }

GUARD_FN_TYPE_DEDUCTION(isSignbitSet);

inline constexpr u8  toggleSignBit(u8 i)  { return u8(-i8(i)); }
inline constexpr u16 toggleSignBit(u16 i) { return u16(-i16(i)); }

GUARD_FN_TYPE_DEDUCTION(toggleSignBit);

inline constexpr u16 u16FromLowAndHi(bool isWord, u8 low, u8 high) {
    u16 ret = 0;
    if (isWord) {
        ret = (u16(high) << 8) | u16(low);
    }
    else {
        if (isSignbitSet(low)) {
            low = toggleSignBit(low);
            ret = u16(low);
            ret = toggleSignBit(ret);
        }
        else {
            ret = u16(low);
        }
    }
    return ret;
}

GUARD_FN_TYPE_DEDUCTION(u16FromLowAndHi);

void safeCastSignedInt(i8 from, i16& to);
void safeCastSignedInt(i8 from, i32& to);
void safeCastSignedInt(i16 from, i32& to);
void safeCastSignedInt(i8 from, i64& to);
void safeCastSignedInt(i16 from, i64& to);
void safeCastSignedInt(i32 from, i64& to);

GUARD_FN_TYPE_DEDUCTION(safeCastSignedInt);

i64 calcShortJmpOffset(i8 loc, i64 start);

GUARD_FN_TYPE_DEDUCTION(calcShortJmpOffset);

} // namespace asm8086
