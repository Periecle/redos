cmake_minimum_required(VERSION 3.10)
project(kernel C)

set(KERNEL_SOURCES
  arch/x86_64/boot.S
  arch/x86_64/crti.S
  arch/x86_64/crtn.S
  arch/x86_64/tty.c
  arch/x86_64/serial.c
  kernel/gdt.c
  kernel/multiboot.c
  kernel/kernel.c
  kernel/paging.c
  kernel/debug.c
  kernel/panic.c
)

# Explicitly treat assembly files as such.
set_source_files_properties(arch/x86_64/boot.S PROPERTIES LANGUAGE ASM)
set_source_files_properties(arch/x86_64/crti.S PROPERTIES LANGUAGE ASM)
set_source_files_properties(arch/x86_64/crtn.S PROPERTIES LANGUAGE ASM)

# Include kernel headers and the libc freestanding headers.
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/libc/include
)

# Create the executable kernel image.
add_executable(redos.kernel ${KERNEL_SOURCES})

# Link against the custom libc library (libk) to resolve functions such as memmove, memset, strlen, printf, etc.
target_link_libraries(redos.kernel PRIVATE libk)

# Specify linker options:
# -T: Use the custom linker script.
# -nostdlib, -nodefaultlibs, -nostartfiles: Prevent inclusion of standard runtime libraries and startup files.
# -lgcc: Link in the gcc support library.
target_link_options(redos.kernel PRIVATE
  "-T${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/linker.ld"
  "-nostdlib"
  "-nodefaultlibs"
  "-nostartfiles"
  "-lgcc"
)

# Define sysroot and boot directories.
set(SYSROOT ${CMAKE_BINARY_DIR}/sysroot)
set(BOOTDIR ${SYSROOT}/boot)

# Custom command: Copy the built kernel into the sysroot boot directory.
add_custom_command(
  OUTPUT ${BOOTDIR}/redos.kernel
  COMMAND ${CMAKE_COMMAND} -E make_directory ${BOOTDIR}
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:redos.kernel> ${BOOTDIR}/redos.kernel
  DEPENDS redos.kernel
  COMMENT "Installing kernel to sysroot boot directory"
)

# Custom target to ensure the kernel is installed.
add_custom_target(install_kernel ALL
  DEPENDS ${BOOTDIR}/redos.kernel
)

# Post-build: Verify multiboot2 compliance.
add_custom_command(TARGET redos.kernel POST_BUILD
  COMMAND grub2-file --is-x86-multiboot2 $<TARGET_FILE:redos.kernel>
  COMMENT "Verifying multiboot2 compliance"
)
