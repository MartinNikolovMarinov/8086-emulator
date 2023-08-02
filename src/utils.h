#pragma once

#include "init_core.h"

namespace asm8086 {

#define GUARD_TEMPLATE_DECLARATION(fnName)                                  \
    template <typename ...Invalid> inline constexpr i32                     \
    fnName() {                                                              \
        static_assert(core::always_false<i32>, "Invalid type for "#fnName); \
        return 0;                                                           \
    }

inline constexpr bool isSignbitSet(u8 i)  { return (i & 0x80) != 0; }
inline constexpr bool isSignbitSet(u16 i) { return (i & 0x8000) != 0; }

GUARD_TEMPLATE_DECLARATION(isSignbitSet)

inline constexpr u8  toggleSignBit(u8 i)  { return u8(-i8(i)); }
inline constexpr u16 toggleSignBit(u16 i) { return u16(-i16(i)); }

GUARD_TEMPLATE_DECLARATION(toggleSignBit)

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

GUARD_TEMPLATE_DECLARATION(u16FromLowAndHi)

} // namespace asm8086
