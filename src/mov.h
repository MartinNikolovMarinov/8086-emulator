#pragma once

#include "init_core.h"
#include "mod.h"

struct MovInst_v1 {
    u8 d : 1;
    u8 w : 1;
    u8 reg : 3;
    u8 rm : 3;
    Mod mod : 2;
    i16 disp : 16;

    void encode(core::str_builder<>& sb) const;
};

struct MovInst_v2 {
    u8 w : 1;
    Mod mod : 2;
    u8 rm : 3;
    i16 disp : 16;
    i16 data : 16; // NOTE: I can't determine if data should be signed or unsigned literal.

    void encode(core::str_builder<>& sb) const;
};

struct MovInst_v3 {
    u8 w : 1;
    u8 reg : 3;
    i16 data : 16; // NOTE: I can't determine if data should be signed or unsigned literal.

    void encode(core::str_builder<>& sb) const;
};

struct MovInst_v4 {
    u8 w : 1;
    u8 d : 1;
    u16 addr : 16;

    void encode(core::str_builder<>& sb) const;
};
