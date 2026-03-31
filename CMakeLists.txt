cmake_minimum_required (VERSION 3.1.3...3.10)
project (zzz C)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release)
endif ()

set (CMAKE_C_STANDARD 99)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"
)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wpedantic -Wall -Wextra")
endif ()

file (GLOB sources "*.c")
add_executable (zzz ${sources})
target_compile_definitions (zzz PUBLIC -D_POSIX_C_SOURCE=200809L)

find_library (MATH_LIBRARY m)
if (MATH_LIBRARY)
    target_link_libraries (zzz PUBLIC ${MATH_LIBRARY})
endif ()

include (GNUInstallDirs)
install (TARGETS zzz DESTINATION ${CMAKE_INSTALL_BINDIR})
