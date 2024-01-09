#pragma once

#include <init_core.h>

namespace asm8086 {

namespace detail {

template <typename TSmaller, typename TBigger>
inline constexpr void _safeCastSignedInt(TSmaller from, TBigger& to) {
    static_assert(sizeof(TSmaller) < sizeof(TBigger), "Invalid cast");
    static_assert(std::is_signed_v<TSmaller> && std::is_signed_v<TBigger>, "Not signed types");

    if (from < 0) {
        to = (-from);
        to = -to;
    }
    else {
        to = TBigger(from);
    }
}

} // detail namespace

constexpr inline void safeCastSignedInt(i8 from, i16& to)  { detail::_safeCastSignedInt(from, to); }
constexpr inline void safeCastSignedInt(i8 from, i32& to)  { detail::_safeCastSignedInt(from, to); }
constexpr inline void safeCastSignedInt(i16 from, i32& to) { detail::_safeCastSignedInt(from, to); }
constexpr inline void safeCastSignedInt(i8 from, i64& to)  { detail::_safeCastSignedInt(from, to); }
constexpr inline void safeCastSignedInt(i16 from, i64& to) { detail::_safeCastSignedInt(from, to); }
constexpr inline void safeCastSignedInt(i32 from, i64& to) { detail::_safeCastSignedInt(from, to); }

constexpr inline bool isSignedBitSet(u16 value) {
    return (value & 0x8000) != 0;
}

constexpr inline bool isSignedBitSet(u8 value) {
    return (value & 0x80) != 0;
}

constexpr inline bool isLowRegister(u8 reg)  { return reg < 0b100; }
constexpr inline bool isHighRegister(u8 reg) { return reg >= 0b100; }

constexpr inline u8 lowPart(u16 v) { return u8(v & 0x00FF); }
constexpr inline u8 highPart(u16 v) { return u8((v & 0xFF00) >> 8); }

constexpr inline u16 combineWord(u8 low, u8 high) { return u16(low) | (u16(high) << 8); }

constexpr inline bool isDirectAddrMode(Mod mod, u8 rm) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::MEMORY_NO_DISPLACEMENT && rm == 0b110;
}

constexpr inline bool isRegToReg(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT;
}

constexpr inline bool isEffectiveAddrCalc(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod != Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT;
}

constexpr inline bool isDisplacementMode(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::MEMORY_8_BIT_DISPLACEMENT ||
           mod == Mod::MEMORY_16_BIT_DISPLACEMENT;
}

constexpr inline bool is8bitDisplacement(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::MEMORY_8_BIT_DISPLACEMENT;
}

constexpr inline bool is16bitDisplacement(Mod mod) {
    if (mod == Mod::NONE_SENTINEL) return false;
    return mod == Mod::MEMORY_16_BIT_DISPLACEMENT;
}

} // namespace asm8086
