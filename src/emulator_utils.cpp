#include "emulator_utils.h"

namespace asm8086 {

namespace {

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

} // namespace

void safeCastSignedInt(i8 from, i16& to)  { _safeCastSignedInt(from, to); }
void safeCastSignedInt(i8 from, i32& to)  { _safeCastSignedInt(from, to); }
void safeCastSignedInt(i16 from, i32& to) { _safeCastSignedInt(from, to); }
void safeCastSignedInt(i8 from, i64& to)  { _safeCastSignedInt(from, to); }
void safeCastSignedInt(i16 from, i64& to) { _safeCastSignedInt(from, to); }
void safeCastSignedInt(i32 from, i64& to) { _safeCastSignedInt(from, to); }

} // namespace asm8086
