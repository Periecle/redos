# cmake/i686-elf-toolchain.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER x86_64-elf-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-red-zone -ffreestanding")
# Prevent CMake from trying to run test executables.
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
