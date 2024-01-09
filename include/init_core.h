#pragma once

// IMPORTANT:
// This file initializes default settings for core and std core. Anywhere the application wants to use somthing from
// core or std core, it must include this file instead of directly including them.
// This is because defining default macros can be done only here.

#undef CORE_DEFAULT_ALLOCATOR
#define CORE_DEFAULT_ALLOCATOR() core::StdAllocator

#include <core.h>

using namespace coretypes;

bool initCore(i32 argc, const char** argv);

// Hashing functions for some core types:

template<> addr_size core::hash(const core::StrView& key);
template<> addr_size core::hash(const i32& key);
template<> addr_size core::hash(const u32& key);

template<> bool core::eq(const core::StrView& a, const core::StrView& b);
template<> bool core::eq(const i32& a, const i32& b);
template<> bool core::eq(const u32& a, const u32& b);

namespace asm8086 {

struct CommandLineArguments {
    char* fileName = nullptr;
    i32 fileNameLen = 0;
    bool execFlag = false;
    bool verboseFlag = false;
    bool dumpMemory = false;
    u32 dumpStart = 0;
    u32 dumpEnd = u32(core::MEGABYTE);
    i32 immValuesFmt = 0;

    bool isVerbose() const { return verboseFlag && !dumpMemory; }
};

extern CommandLineArguments cmdArgs;

} // namespace asm8086
