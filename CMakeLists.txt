cmake_minimum_required (VERSION 3.1.3...3.10)
project (zzz C)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release)
endif ()

set (CMAKE_C_STANDARD 99)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"
)
    if (${CMAKE_BUILD_TYPE} MATCHES "Release|MinSizeRel")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
    endif ()
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wpedantic -Wall -Wextra")
endif ()

add_executable (zzz "zzz.c")
target_compile_definitions (zzz PUBLIC -D_POSIX_C_SOURCE=200809L)

include (GNUInstallDirs)
install (TARGETS zzz DESTINATION ${CMAKE_INSTALL_BINDIR})
