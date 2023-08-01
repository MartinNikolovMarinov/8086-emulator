#pragma once

#include "init_core.h"
#include "mod.h"

struct MovInst_v3 {
    u8 w : 1;
    u8 reg : 3;
    u16 data : 16;

    void encode(core::str_builder<>& sb) const;
};

struct MovInst_v4 {
    u8 w : 1;
    u8 d : 1;
    u16 addr : 16;

    void encode(core::str_builder<>& sb) const;
};
