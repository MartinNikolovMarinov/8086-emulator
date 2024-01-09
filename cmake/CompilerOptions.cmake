macro(generate_common_flags
      common_flags_varname common_flags
      debug_flags_varname debug_flags
      release_flags_varname release_flags)

    set(local_common_flags "${common_flags}")
    set(local_debug_flags "${debug_flags}")
    set(local_release_flags "${release_flags}")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
       CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
       CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")

        set(local_common_flags ${local_common_flags}
            -Wall -Wextra -Wfatal-errors -Wconversion -Wpedantic
            -Wshadow -Wold-style-cast -Wdouble-promotion -Wswitch-enum -Wundef -Wcast-align -Wsign-conversion
            -Wmisleading-indentation -Woverloaded-virtual -Wnon-virtual-dtor
            -Wdisabled-optimization # warn if the compailer disables requestd level of optimization
            -Wno-unknown-pragmas -Wno-unused-function -Wno-variadic-macros
        )
        set(local_debug_flags ${local_debug_flags} -g -O0 -Wnull-dereference)
        set(local_release_flags ${local_release_flags} -O3)

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

        set(local_common_flags ${local_common_flags}
            /MP
            -Wall -W4 -WX
            -nologo -FC
            /fastfail
            /w14254 # Warns if more than one operator conversion is applied to get from one type to another (useful for finding implicit conversions that might be unintended).
            /w14263 # Warns if a member function hides a base class virtual function (similar to -Woverloaded-virtual in GCC/Clang).
            /w14265 # Warns if a class is derived from two or more classes that are not interfaces.
            /w14287 # Warns for 'unsigned' char, short, int or long types in a conditional expression, which can lead to unexpected behaviors.
            /w14296 # Warns if a constructor is declared without specifying either 'explicit' or 'implicit'.
            /w14311 # Warns if a catch block has been written to catch C++ exceptions by value rather than by reference.
        )
        set(local_debug_flags ${local_debug_flags} -Zi -Od)
        set(local_release_flags ${local_release_flags} -O2)
    else()
        message(FATAL_ERROR "Unsupported compiler")
        return()
    endif()

    set(${common_flags_varname} ${local_common_flags})
    set(${debug_flags_varname} ${local_debug_flags})
    set(${release_flags_varname} ${local_release_flags})

endmacro()
