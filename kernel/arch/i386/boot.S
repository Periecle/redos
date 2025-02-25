/*  boot.S - bootstrap the kernel */
/*  Copyright (C) 1999, 2001, 2010  Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define ASM_FILE        1
#include <multiboot2.h>

/*  C symbol format. HAVE_ASM_USCORE is defined by configure. */
#ifdef HAVE_ASM_USCORE
# define EXT_C(sym)                     _ ## sym
#else
# define EXT_C(sym)                     sym
#endif

/*  The size of our stack (16KB). */
#define STACK_SIZE                      0x4000

/* Define constants for the higher half kernel */
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 22)

/*  The flags for the Multiboot header. */
#ifdef __ELF__
# define AOUT_KLUDGE 0
#else
# define AOUT_KLUDGE MULTIBOOT_AOUT_KLUDGE
#endif

/* This code must be placed in the .bootstrap_text section to ensure
   it's loaded at the correct physical address before paging is enabled */
.section .bootstrap_text, "ax"
.globl  start, _start
start:
_start:
        jmp     multiboot_entry

        /*  Align 64 bits boundary. */
        .align  8

        /*  Multiboot header. */
multiboot_header:
        /*  magic */
        .long   MULTIBOOT2_HEADER_MAGIC
        /*  ISA: i386 */
        .long   MULTIBOOT_ARCHITECTURE_I386
        /*  Header length. */
        .long   multiboot_header_end - multiboot_header
        /*  checksum */
        .long   -((MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386) + (multiboot_header_end - multiboot_header))
#ifndef __ELF__
address_tag_start:
        .short MULTIBOOT_HEADER_TAG_ADDRESS
        .short MULTIBOOT_HEADER_TAG_OPTIONAL
        .long address_tag_end - address_tag_start
        /*  header_addr */
        .long   multiboot_header
        /*  load_addr */
        .long   _start
        /*  load_end_addr */
        .long   _edata
        /*  bss_end_addr */
        .long   _end
address_tag_end:
entry_address_tag_start:
        .short MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
        .short MULTIBOOT_HEADER_TAG_OPTIONAL
        .long entry_address_tag_end - entry_address_tag_start
        /*  entry_addr */
        .long multiboot_entry
entry_address_tag_end:
#endif /*  __ELF__ */
        .short MULTIBOOT_HEADER_TAG_END
        .short 0
        .long 8
multiboot_header_end:

/* Paging structures - placed in .bootstrap_data */
.section .bootstrap_data, "aw"
.align 4096
boot_page_directory:
        .space 4096

boot_page_table1:
        .space 4096

/* Temporary stack for bootstrapping */
.section .bootstrap_bss, "aw", @nobits
.align 16
.skip STACK_SIZE
boot_stack:

/* Entry point */
.section .bootstrap_text
multiboot_entry:
        /* Initialize the stack pointer (physical address) */
        movl    $(boot_stack), %esp

        /* Save multiboot info */
        movl    %ebx, %edi  /* Save multiboot info pointer */
        movl    %eax, %esi  /* Save multiboot magic */

        /* Print a message to indicate we're starting */
        movl    $boot_message, %ebx
        call    early_print_string

        /* Set up the page tables */
        call    setup_page_tables

        /* Load the page directory */
        movl    $(boot_page_directory), %ecx
        movl    %ecx, %cr3

        /* Enable paging */
        movl    %cr0, %ecx
        orl     $0x80000000, %ecx  /* Set the paging bit in CR0 */
        movl    %ecx, %cr0

        /* Jump to higher half kernel */
        lea     higher_half, %ecx
        jmp     *%ecx

/* Setup initial page tables */
setup_page_tables:
        /* Clear page directory */
        movl    $(boot_page_directory), %edi
        xorl    %eax, %eax
        movl    $1024, %ecx
        cld
        rep stosl

        /* First, identity map the first 4MB */
        movl    $(boot_page_table1), %edi
        movl    $0x00000003, %eax  /* Present, writable */
        movl    $1024, %ecx        /* Map 1024 pages = 4MB */

1:      stosl
        addl    $4096, %eax        /* Move to next page (4KB) */
        loop    1b

        /* Map the page table at both 0x00000000 and 0xC0000000 */
        movl    $(boot_page_table1 + 0x003), %eax  /* Present, writable */
        movl    %eax, boot_page_directory          /* First 4MB identity mapped */
        movl    %eax, boot_page_directory + (KERNEL_PAGE_NUMBER * 4)  /* Higher half mapping */

        movl    $page_tables_setup, %ebx
        call    early_print_string
        ret

/* This is our early stage printing function - direct to VGA */
early_print_string:
        /* Input: ebx = string pointer */
        movl    $0xB8000, %edx     /* VGA buffer physical address */
        movl    $0, %ecx           /* String index */

1:      movb    (%ebx, %ecx), %al  /* Load character */
        testb   %al, %al
        jz      2f                  /* If null terminator, exit */

        movb    %al, (%edx)         /* Write character */
        movb    $0x0F, 1(%edx)      /* White on black attribute */
        addl    $2, %edx            /* Next character position */
        incl    %ecx                /* Next character in string */
        jmp     1b

2:      ret

/* Here we enter the higher half kernel code */
.section .text
higher_half:
        /* Now we're in the higher half, using virtual addresses */

        /* Set up virtual stack */
        addl    $KERNEL_VIRTUAL_BASE, %esp

        /* Store multiboot info for kernel (saved earlier in edi/esi) */
        pushl   %edi  /* Multiboot info pointer */
        pushl   %esi  /* Multiboot magic */

        /* Print a message that we've reached the higher half */
        movl    $higher_half_message, %ebx
        call    early_virt_print_string

        /* Reset EFLAGS. */
        pushl   $0
        popf

        /* Validate MultiBoot 2 information */
        call    EXT_C(validate_boot)

        /* Setup Global Descriptor Table */
        call    EXT_C(setup_gdt)

        /* Init Global Constructors */
        call    EXT_C(_init)

        /* Initialize the terminal (using virtual addresses) */
        call    EXT_C(terminal_initialize)

        /* Run kernel main */
        call    EXT_C(kernel_main)

        /* Halt. */
        pushl   $halt_message
        call    EXT_C(printf)

loop:   hlt
        jmp     loop

/* Early printing function for virtual addresses */
early_virt_print_string:
        /* Input: ebx = string pointer */
        movl    $(0xB8000 + KERNEL_VIRTUAL_BASE), %edx  /* VGA buffer virtual address */
        movl    $0, %ecx           /* String index */

1:      movb    (%ebx, %ecx), %al  /* Load character */
        testb   %al, %al
        jz      2f                  /* If null terminator, exit */

        movb    %al, (%edx)         /* Write character */
        movb    $0x0E, 1(%edx)      /* Yellow on black attribute */
        addl    $2, %edx            /* Next character position */
        incl    %ecx                /* Next character in string */
        jmp     1b

2:      ret

/* Strings */
boot_message:
        .asciz  "RedOS: Booting in physical mode, setting up paging..."

page_tables_setup:
        .asciz  "RedOS: Page tables set up, enabling paging..."

higher_half_message:
        .asciz  "RedOS: Successfully jumped to higher half kernel at 0xC0000000!"

halt_message:
        .asciz  "System halted."
