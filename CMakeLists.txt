cmake_minimum_required(VERSION 3.2)

project(main)

# Global CMake options:
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json

# Create executable:
add_executable(${PROJECT_NAME}
    main.cpp

    src/init_core.cpp
    src/init_core.h
    src/inst_8086.cpp
    src/inst_8086.h
    src/mod.h
    src/opcode.cpp
    src/opcode.h
    src/utils.h
)

# BEGIN Compile Options

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(mm_flags -std=c++17 -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-function -Wfatal-errors)
    set(mm_core_debug_flags -O0 -g -ggdb)
    set(mm_core_release_flags)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(mm_flags
        /std:c++17
        /nologo /FC
        /W4
        /EHsc
    )
    set(mm_core_debug_flags /O0 /Zi /Od /Oi)
    set(mm_core_release_flags /O2)
else()
    message(FATAL_ERROR "Unsupported compiler")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE ${mm_flags} ${mm_core_debug_flags})
else()
    target_compile_options(${PROJECT_NAME} PRIVATE ${mm_flags} ${mm_core_release_flags})
endif()

# END Compile Options

# Definitions:
target_compile_definitions(${PROJECT_NAME} PRIVATE
    -DPATH_TO_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

# fmt library:
target_link_libraries(${PROJECT_NAME} PRIVATE fmt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt/include)

# Core library:
option(CORE_DEBUG "" ON)
option(CORE_ASSERT_ENABLED "" ON)
option(CORE_LIBRARY_SHARED "" ON)
option(CORE_BUILD_TESTS "" OFF)
option(RUN_COMPILETIME_TESTS "" OFF)
option(CORE_LIBRARY_SHARED "" ON)
target_link_libraries(${PROJECT_NAME} PRIVATE core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/core ${CMAKE_CURRENT_BINARY_DIR}/core)
