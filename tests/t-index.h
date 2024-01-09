#pragma once

#include <init_core.h>
#include <logger.h>
#include <decoder.h>
#include <emulator.h>

#include <iostream>

using namespace asm8086;

i32 runDecoderTestsSuite();
i32 runEmulatorTestsSuite();
i32 runAllTests();
