cmake_minimum_required(VERSION 3.10)
project(RedosOS C ASM)

# Global compile options for a freestanding build.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -ffreestanding -Wall -Wextra -fstack-protector-strong")

# Set output directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include subdirectories for projects.
add_subdirectory(libc)
add_subdirectory(kernel)

# Define installation directories (for our sysroot).
set(SYSROOT ${CMAKE_BINARY_DIR}/sysroot)
set(BOOTDIR ${SYSROOT}/boot)

# ISO build settings.
set(ISODIR ${CMAKE_BINARY_DIR}/isodir)
set(ISO_FILE ${CMAKE_BINARY_DIR}/redos.iso)

# Custom target: Build ISO image using grub2-mkrescue.
add_custom_target(iso ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${ISODIR}/boot/grub
  COMMAND ${CMAKE_COMMAND} -E copy ${BOOTDIR}/redos.kernel ${ISODIR}/boot/redos.kernel
  # Write a simple GRUB configuration file.
  COMMAND ${CMAKE_COMMAND} -E echo "menuentry \"redos\" { multiboot2 /boot/redos.kernel }" > ${ISODIR}/boot/grub/grub.cfg
  COMMAND grub2-mkrescue -o ${ISO_FILE} ${ISODIR}
  DEPENDS install_kernel
  COMMENT "Generating bootable ISO image"
)

# Custom target: Run QEMU.
add_custom_target(qemu
  COMMAND qemu-system-i386 -cdrom ${ISO_FILE}
  DEPENDS iso
  COMMENT "Launching QEMU with the generated ISO"
)
