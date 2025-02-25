# cmake/i686-elf-toolchain.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_C_COMPILER i686-elf-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding")
# Prevent CMake from trying to run test executables.
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
