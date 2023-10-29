cmake_minimum_required(VERSION 3.2)

project(main)

# Global CMake options:
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json

# Application source files
set(SOURCES
    src/init_core.cpp
    src/init_core.h

    src/emulator_utils.h
    src/emulator.cpp
    src/emulator.h
    src/opcode.cpp
    src/opcode.h
)

# Create executable:
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

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
target_link_libraries(${PROJECT_NAME} PRIVATE core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/core ${CMAKE_CURRENT_BINARY_DIR}/core)

# Tests:
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(run_tests test/run_tests.cpp ${SOURCES})
    target_compile_options(run_tests PRIVATE ${mm_flags} ${mm_core_debug_flags})
    target_compile_definitions(run_tests PRIVATE
        -DPATH_TO_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data"
    )
    target_link_libraries(run_tests PRIVATE fmt)
    target_include_directories(run_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt/include
    )
    target_link_libraries(run_tests PRIVATE core)
endif()
