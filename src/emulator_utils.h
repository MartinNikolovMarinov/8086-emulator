#pragma once

#include "init_core.h"

namespace asm8086 {

void safeCastSignedInt(i8 from, i16& to);
void safeCastSignedInt(i8 from, i32& to);
void safeCastSignedInt(i16 from, i32& to);
void safeCastSignedInt(i8 from, i64& to);
void safeCastSignedInt(i16 from, i64& to);
void safeCastSignedInt(i32 from, i64& to);

GUARD_FN_TYPE_DEDUCTION(safeCastSignedInt);

} // namespace asm8086
