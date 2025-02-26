/* 64-bit Kernel Linker Script */
ENTRY(_start)

/* Define kernel addresses */
KERNEL_VIRTUAL_BASE = 0xFFFFFFFF80000000;
KERNEL_PHYSICAL_BASE = 0x00100000; /* 1MB */

SECTIONS
{
    /* Start at 1MB physical */
    . = KERNEL_PHYSICAL_BASE;

    kernel_physical_start = .;

    /* Multiboot header must be at the start */
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }

    /* 32-bit bootstrap code */
    .bootstrap ALIGN(4K) : {
        *(.bootstrap)
    }

    /* Bootstrap data and variables */
    .bootstrap_data ALIGN(4K) : {
        *(.bootstrap_data)
        *(.bootstrap_vars)
        *(.data .data.*)
    }

    /* Page tables */
    .bootstrap_tables ALIGN(4K) : {
        *(.bootstrap_tables)
    }

    /* Bootstrap stack */
    .bootstrap_stack ALIGN(16) : {
        *(.bootstrap_stack)
    }

    /* Transition to higher half kernel */
    . += KERNEL_VIRTUAL_BASE;
    kernel_virtual_start = .;

    /* 64-bit executable code */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE) {
        *(.text .text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE) {
        *(.rodata .rodata.*)
        *(.eh_frame)
    }

    /* Read-write data */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE) {
        *(.data)
    }

    /* Uninitialized data */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss .bss.*)
    }

    /* Clean up */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }

    /* Record kernel end addresses */
    kernel_physical_end = . - KERNEL_VIRTUAL_BASE;
    kernel_virtual_end = .;
}
