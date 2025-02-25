cmake_minimum_required(VERSION 3.10)
project(libc C)

set(LIBC_SOURCES
  stdio/printf.c
  stdio/putchar.c
  stdio/puts.c
  stdlib/abort.c
  string/memcmp.c
  string/memcpy.c
  string/memmove.c
  string/memset.c
  string/strlen.c
  stdlib/stack_guard.c
)

# Create a static library target.
add_library(libk STATIC ${LIBC_SOURCES})

# Include libc headers.
target_include_directories(libk PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel/include
)

# Define a macro to mark the build as for libc.
target_compile_definitions(libk PUBLIC __is_libc)
