#pragma once

#include "init_core.h"

namespace asm8086 {

enum Mod : u8 {
    MOD_MEMORY_NO_DISPLACEMENT               = 0b00,
    MOD_MEMORY_8_BIT_DISPLACEMENT            = 0b01,
    MOD_MEMORY_16_BIT_DISPLACEMENT           = 0b10,
    MOD_REGISTER_TO_REGISTER_NO_DISPLACEMENT = 0b11,
};

} // namespace asm8086
