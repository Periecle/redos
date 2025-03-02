# boot.S - Minimal 64-bit kernel entry point
#define ASM_FILE 1
#include <multiboot2.h>

# Constants
.set KERNEL_VIRTUAL_BASE, 0xFFFFFFFF80000000
.set PAGE_SIZE, 4096
.set PAGE_PRESENT, 0x1
.set PAGE_WRITE, 0x2
.set PAGE_HUGE, 0x80

# Debug constants
.set VGA_BUFFER, 0xB8000
.set DEBUG_COLOR, 0x2F  # Green on black

# Multiboot header - must be first in binary!
.section .multiboot, "a"
.align 8
multiboot_header:
    # Multiboot2 header fields
    .long MULTIBOOT2_HEADER_MAGIC           # Magic number
    .long MULTIBOOT_ARCHITECTURE_I386       # Architecture (i386)
    .long multiboot_header_end - multiboot_header  # Header length
    # Checksum: -(magic + architecture + header_length) mod 2^32
    .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))

    # Information request tag - required to get memory map
    .short MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
    .short 0        # Flags - must be zero
    .long 16        # Tag size including header (8) + two 4-byte entries
    .long MULTIBOOT_TAG_TYPE_MMAP           # Request memory map
    .long MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  # Request bootloader name

    # End tag
    .short MULTIBOOT_HEADER_TAG_END
    .short 0        # Flags - must be zero
    .long 8         # Tag size (8 bytes)
multiboot_header_end:

# Allocate initial bootstrap stack
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
    .skip 16384  # 16 KiB
stack_top:

# Reserve space for page tables
.section .bootstrap_page_tables, "aw", @nobits
.align 4096
pml4_table:
    .skip 4096
pdpt_table:
    .skip 4096
pd_table:
    .skip 4096

# Debug strings
.section .rodata
debug_msg: .asciz "RedOS 64-bit is booting..."

# Kernel entry point
.section .bootstrap, "ax"
.code32
.global _start
_start:
    # Clear direction flag
    cld

    # Save multiboot info
    movl %ebx, mb_info_ptr - KERNEL_VIRTUAL_BASE
    movl %eax, mb_magic - KERNEL_VIRTUAL_BASE

    # Set up stack
    movl $(stack_top - KERNEL_VIRTUAL_BASE), %esp

    # Write debug message to VGA (direct write to verify we're running)
    movl $VGA_BUFFER, %edi
    movl $(debug_msg - KERNEL_VIRTUAL_BASE), %esi
    movb $DEBUG_COLOR, %bl  # Use 8-bit register for color
write_char:
    movb (%esi), %al
    testb %al, %al
    jz setup_cpu
    movb %bl, %ah  # Move color to ah
    movw %ax, (%edi)
    addl $1, %esi
    addl $2, %edi
    jmp write_char

setup_cpu:
    # Enable PAE
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4

    # Set long mode bit in EFER MSR
    movl $0xC0000080, %ecx
    rdmsr
    orl $0x100, %eax
    wrmsr

    # Set up paging structures
    # Clear tables first
    movl $(pml4_table - KERNEL_VIRTUAL_BASE), %edi
    xorl %eax, %eax
    movl $3072, %ecx
    rep stosl

    # PML4 Entry 0 (identity mapping)
    movl $(pdpt_table - KERNEL_VIRTUAL_BASE + PAGE_PRESENT + PAGE_WRITE), %eax
    movl %eax, (pml4_table - KERNEL_VIRTUAL_BASE)

    # PML4 Entry 511 (higher half mapping)
    movl $(pdpt_table - KERNEL_VIRTUAL_BASE + PAGE_PRESENT + PAGE_WRITE), %eax
    movl %eax, (pml4_table - KERNEL_VIRTUAL_BASE + 511*8)

    # PDPT Entry 0 (identity mapping)
    movl $(pd_table - KERNEL_VIRTUAL_BASE + PAGE_PRESENT + PAGE_WRITE), %eax
    movl %eax, (pdpt_table - KERNEL_VIRTUAL_BASE)

    # PDPT Entry 510 (higher half mapping)
    movl $(pd_table - KERNEL_VIRTUAL_BASE + PAGE_PRESENT + PAGE_WRITE), %eax
    movl %eax, (pdpt_table - KERNEL_VIRTUAL_BASE + 510*8)

    # Map first 2MB with a huge page
    movl $(PAGE_PRESENT | PAGE_WRITE | PAGE_HUGE), %eax
    movl %eax, (pd_table - KERNEL_VIRTUAL_BASE)

    # Map second 2MB with a huge page (for VGA buffer)
    movl $(0x200000 | PAGE_PRESENT | PAGE_WRITE | PAGE_HUGE), %eax
    movl %eax, (pd_table - KERNEL_VIRTUAL_BASE + 8)

    # Load PML4 into CR3
    movl $(pml4_table - KERNEL_VIRTUAL_BASE), %eax
    movl %eax, %cr3

    # Enable paging and protected mode
    movl %cr0, %eax
    orl $0x80000001, %eax  # Set PG and PE bits
    movl %eax, %cr0

    # Load 64-bit GDT
    lgdt (gdt64_ptr - KERNEL_VIRTUAL_BASE)

    # Jump to 64-bit code
    ljmp $0x08, $(long_mode_entry - KERNEL_VIRTUAL_BASE)

.align 16
gdt64:
    # Null descriptor
    .quad 0
    # Code segment descriptor
    .quad 0x00AF9A000000FFFF  # Long mode code
    # Data segment descriptor
    .quad 0x00AF92000000FFFF  # Long mode data
gdt64_ptr:
    .word gdt64_ptr - gdt64 - 1  # Limit
    .long gdt64 - KERNEL_VIRTUAL_BASE  # Base

.section .bootstrap_data, "aw", @nobits
mb_info_ptr:
    .long 0
mb_magic:
    .long 0

.section .text
.code64
long_mode_entry:
    # Set up segment registers - individual instructions
    movq $0x10, %rax  # Use 64-bit register
    movw %ax, %ds     # Then move lower 16 bits to segment registers
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    # Write to VGA directly in long mode to verify we got here
    movq $VGA_BUFFER, %rdi
    addq $160, %rdi  # Move to next line
    movq $0x2F582F582F582F58, %rax  # "XXXX" in green on black
    movq %rax, (%rdi)

    # Set up higher-half stack
    movabsq $stack_top, %rsp

    # Clear the frame pointer
    xorq %rbp, %rbp

    # Prepare arguments for kernel_main
    movl mb_magic(%rip), %edi
    movl mb_info_ptr(%rip), %esi

    # Check if multiboot info is a physical address
    movabsq $0xFFFFFFFF00000000, %rax
    testq %rax, %rsi
    jnz 1f

    # Convert to higher-half address if it's a physical address
    movabsq $KERNEL_VIRTUAL_BASE, %rax
    addq %rax, %rsi
1:
    # Call kernel_main
    movabsq $kernel_main, %rax
    callq *%rax

    # Kernel should never return, but if it does:
halt:
    cli
    hlt
    jmp halt
