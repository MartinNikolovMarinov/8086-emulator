if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR
   CMAKE_SYSTEM_NAME STREQUAL "CYGWIN" OR
   CMAKE_SYSTEM_NAME STREQUAL "MSYS" OR
   CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
    set(OS "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OS "darwin")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OS "linux")
else()
    set(OS "unsupported")
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}!")
endif()
