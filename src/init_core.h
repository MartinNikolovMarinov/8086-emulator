#pragma once

// IMPORTANT:
// This file initializes default settings for core and std core. Anywhere the application wants to use somthing from
// core or std core, it must include this file instead of directly including them.
// This is because defining default macros can be done only here.

#include <std/allocators/alloc_std_stats.h>

using namespace coretypes;

static core::std_stats_allocator g_stdAlloc;
struct std_allocator_static {
    static void* alloc(addr_size size) noexcept;

    static void free(void* ptr) noexcept;

    static addr_size used_mem() noexcept;

    static const char* allocator_name() noexcept;
};

#undef CORE_DEFAULT_ALLOCATOR
#define CORE_DEFAULT_ALLOCATOR() std_allocator_static

#include <core.h>
#include <std/core.h>

struct command_line_args {
    char* fileName = nullptr;
    i32 fileNameLen = 0;
    bool execFlag = false;
    bool verboseFlag = false;
    i32 immValuesFmt = 0;
};

command_line_args initCore(i32 argc, const char** argv);

#include <fmt/core.h>
#include <fmt/color.h>
