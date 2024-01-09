cmake_minimum_required(VERSION 3.3)

set(executable_name emulator)
string(TOUPPER ${executable_name} executable_name_uppercase)
set(main_file main.cpp)

project(${executable_name} VERSION 0.0.1)

# Standard requirenments:

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json

# Force cmake to put all libraries and executables in the same directory, so that special compilers can find them:

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Detect Build Type

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    set(${executable_name_uppercase}_DEBUG OFF)
else()
    set(${executable_name_uppercase}_DEBUG ON)
endif()

# Options:

option(${executable_name_uppercase}_BUILD_TESTS "Build tests." OFF)

# Includes:

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/cmake"
)

include(DetectOS)
include(CompilerOptions)

# Default compiler options macro:

macro(target_set_default_flags target)

    if(OS STREQUAL "linux")
        set(common_flags "-std=c++20")
        set(debug_flags "")
        set(release_flags "")
    elseif(OS STREQUAL "darwin")
        set(common_flags "-std=c++20")
        set(debug_flags "")
        set(release_flags "")
    elseif(OS STREQUAL "windows")
        set(common_flags "/std:c++20")
        set(debug_flags "")
        set(release_flags "")
    endif()

    generate_common_flags(
        common_flags "${common_flags}"
        debug_flags "${debug_flags}"
        release_flags "${release_flags}"
    )

    if(${executable_name_uppercase}_DEBUG)
        target_compile_options(${executable_name} PRIVATE ${common_flags} ${debug_flags})
    else()
        target_compile_options(${executable_name} PRIVATE ${common_flags} ${release_flags})
    endif()

endmacro()

# Create executable:

add_subdirectory(lib/core)

set(src_files

   src/init_core.cpp
   src/logger.cpp
   src/opcodes.cpp
   src/decoder.cpp
   src/emulator.cpp
)

add_executable(${executable_name} ${main_file} ${src_files})

# Unify symbol visibility across all platforms:
set_target_properties(${executable_name} PROPERTIES
    C_VISIBILITY_PRESET hidden
    C_VISIBILITY_INLINE_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_INLINE_PRESET hidden
)

target_include_directories(${executable_name} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(${executable_name} PUBLIC
    ${executable_name_uppercase}="$<BOOL:${executable_name_uppercase}_DEBUG>"
    ${executable_name_uppercase}_BINARY_PATH="${CMAKE_BINARY_DIR}/"
)

target_link_libraries(${executable_name} PUBLIC
    core
)

target_set_default_flags(${executable_name})

# Create test executable:

if (${executable_name_uppercase}_BUILD_TESTS)

    set(test_files
        tests/t-index.cpp
        tests/t-decoder.cpp
        tests/t-emulator.cpp
    )

    add_executable(${executable_name}_test test_${main_file} ${test_files} ${src_files})

    target_include_directories(${executable_name}_test PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_compile_definitions(${executable_name}_test PUBLIC
        ${executable_name_uppercase}="$<BOOL:${executable_name_uppercase}_DEBUG>"
        ${executable_name_uppercase}_BINARY_PATH="${CMAKE_BINARY_DIR}/"
    )

    target_link_libraries(${executable_name}_test PUBLIC
        core
    )

    target_set_default_flags(${executable_name}_test)

    # Add the test
    enable_testing()
    add_test(NAME ${executable_name}_test COMMAND ${executable_name}_test)

endif()
