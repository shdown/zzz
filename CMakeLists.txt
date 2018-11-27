cmake_minimum_required (VERSION 3.0.0)
project (zzz C)

set (CMAKE_C_STANDARD 99)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"
)
    if (${CMAKE_BUILD_TYPE} MATCHES "Release|MinSizeRel")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
    endif ()
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Wextra -Wcast-qual -Wredundant-decls -Wshadow -Winit-self")
endif ()

add_executable (zzz "zzz.c")
target_compile_definitions (zzz PUBLIC -D_POSIX_C_SOURCE=200809L)

include (GNUInstallDirs)
install (TARGETS zzz DESTINATION ${CMAKE_INSTALL_BINDIR})
