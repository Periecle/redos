# boot.S - 64-bit kernel entry point and bootstrap
#define ASM_FILE 1
#include <multiboot2.h>

# Constants
.set KERNEL_VIRTUAL_BASE, 0xFFFFFFFF80000000
.set PAGE_SIZE,           4096
.set PAGE_PRESENT,        (1 << 0)
.set PAGE_WRITE,          (1 << 1)
.set PAGE_USER,           (1 << 2)
.set PAGE_SIZE_BIT,       (1 << 7)

# Multiboot2 header for GRUB
.section .multiboot, "a"
.align 8
multiboot_header:
    .long   MULTIBOOT2_HEADER_MAGIC
    .long   MULTIBOOT_ARCHITECTURE_I386
    .long   multiboot_header_end - multiboot_header
    .long   -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))

    # Information request tag
    .short  MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
    .short  0
    .long   12  # Tag size
    .long   MULTIBOOT_TAG_TYPE_MMAP
    .long   MULTIBOOT_TAG_TYPE_CMDLINE
    .long   MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME

    # End tag
    .short  MULTIBOOT_HEADER_TAG_END
    .short  0
    .long   8
multiboot_header_end:

# 32-bit entry point
.section .bootstrap, "ax"
.code32
.global _start
_start:
    # Disable interrupts
    cli

    # Save multiboot info using RIP-relative addressing
    movl    %ebx, multiboot_info_ptr(%ebx)
    movl    %eax, multiboot_magic(%ebx)

    # Setup temporary stack
    movl    $boot_stack_top, %esp

    # Check multiboot magic
    cmpl    $MULTIBOOT2_BOOTLOADER_MAGIC, %eax
    jne     multiboot_error

    # Enable PAE and SSE
    movl    %cr4, %eax
    orl     $0x620, %eax    # PAE (bit 5), OSFXSR (bit 9), OSXMMEXCPT (bit 10)
    movl    %eax, %cr4

    # Setup long mode
    call    check_cpuid
    call    check_long_mode

    # Setup page tables
    call    setup_page_tables

    # Load 64-bit GDT
    lgdt    gdt64_ptr

    # Enable long mode and paging
    movl    $0xC0000080, %ecx  # EFER MSR
    rdmsr
    orl     $0x100, %eax    # LME bit
    wrmsr

    # Enable paging and protected mode
    movl    %cr0, %eax
    orl     $0x80000001, %eax   # PG and PE bits
    movl    %eax, %cr0

    # Far jump to 64-bit code with proper segment
    ljmp    $0x08, $long_mode_start

# Check for CPUID support
check_cpuid:
    # Try to flip ID bit in EFLAGS
    pushfl
    popl    %eax
    movl    %eax, %ecx
    xorl    $0x200000, %eax
    pushl   %eax
    popfl
    pushfl
    popl    %eax
    xorl    %ecx, %eax
    testl   $0x200000, %eax
    jz      .no_cpuid
    ret

.no_cpuid:
    # Use RIP-relative addressing for error message
    movl    $no_cpuid_msg, %esi
    call    print_error
    jmp     halt

# Check for long mode support
check_long_mode:
    movl    $0x80000000, %eax
    cpuid
    cmpl    $0x80000001, %eax
    jb      .no_long_mode

    movl    $0x80000001, %eax
    cpuid
    testl   $0x20000000, %edx  # Long mode bit
    jz      .no_long_mode
    ret

.no_long_mode:
    # Use RIP-relative addressing for error message
    movl    $no_long_mode_msg, %esi
    call    print_error
    jmp     halt

# Setup 4-level paging for 64-bit
setup_page_tables:
    # Clear page tables
    movl    $pml4_table, %edi
    xorl    %eax, %eax
    movl    $3072, %ecx  # 3 pages (PML4, PDPT, PD)
    rep     stosb

    # PML4[0] and PML4[511] point to same PDPT
    movl    $pdpt_table, %eax
    orl     $(PAGE_PRESENT | PAGE_WRITE), %eax
    movl    %eax, pml4_table
    movl    %eax, pml4_table + (511 * 8)

    # PDPT[0] and PDPT[510] point to PD
    movl    $pd_table, %eax
    orl     $(PAGE_PRESENT | PAGE_WRITE), %eax
    movl    %eax, pdpt_table
    movl    %eax, pdpt_table + (510 * 8)

    # Map first 2MB with huge pages
    movl    $pd_table, %edi
    movl    $(PAGE_PRESENT | PAGE_WRITE | PAGE_SIZE_BIT), %eax
    movl    %eax, (%edi)  # First 2MB identity mapped

    # Load CR3 with PML4 address
    movl    $pml4_table, %eax
    movl    %eax, %cr3

    ret

# Error handling and debug output
print_error:
    movl    $0xb8000, %edi
    movl    $0x4f4f4f4f, %eax  # Red background
1:
    movb    (%esi), %bl
    testb   %bl, %bl
    jz      2f
    movw    %ax, (%edi)
    incl    %esi
    addl    $2, %edi
    jmp     1b
2:
    ret

# 64-bit long mode entry
.section .text
.code64
long_mode_start:
    # Setup segment registers
    movq    $0x10, %rax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss

    # Zero out the base pointer
    xorq    %rbp, %rbp

    # Restore multiboot parameters
    movq    multiboot_magic(%rip), %rdi
    movq    multiboot_info_ptr(%rip), %rsi

    # Jump to higher half kernel
    movabs  $kernel_main, %rax
    jmpq    *%rax

# Halt the system
halt:
    cli
    hlt
    jmp     halt

# 64-bit GDT descriptor
.align 16
gdt64:
    .quad   0  # Null descriptor
    .quad   0x00AF9A000000FFFF  # 64-bit code segment
    .quad   0x00AF92000000FFFF  # 64-bit data segment

gdt64_ptr:
    .word   (gdt64_ptr - gdt64 - 1)  # Limit
    .quad   gdt64  # Base

# Message strings
.section .rodata
no_cpuid_msg:     .asciz "No CPUID support"
no_long_mode_msg: .asciz "Long mode not supported"
multiboot_error:  .asciz "Not loaded by Multiboot2"

# Multiboot info storage
.section .data
multiboot_magic:      .quad 0
multiboot_info_ptr:   .quad 0

# Page tables (4KB aligned)
.section .bootstrap_tables, "aw", @nobits
.align 4096
pml4_table:   .space 4096
pdpt_table:   .space 4096
pd_table:     .space 4096

# Bootstrap stack
.section .bootstrap_stack, "aw", @nobits
.align 16
    .space 16384  # 16 KB stack
boot_stack_top:
