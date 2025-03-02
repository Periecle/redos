/* Linker script for 64-bit kernel */
ENTRY(_start)

/* Define the kernel addressing scheme */
KERNEL_VIRTUAL_BASE = 0xFFFFFFFF80000000;
KERNEL_PHYSICAL_BASE = 0x00100000; /* 1MB */

SECTIONS
{
    /* Start at 1MB physical */
    . = KERNEL_PHYSICAL_BASE;

    /* Physical start marker */
    kernel_physical_start = .;

    /* IMPORTANT: Multiboot header must be first */
    .multiboot : {
        KEEP(*(.multiboot))
    }

    /* Bootstrap code section (32-bit) */
    .bootstrap : {
        *(.bootstrap)
        *(.bootstrap_rodata)
        *(.bootstrap_data)
    }

    /* Bootstrap page tables section */
    .bootstrap_page_tables ALIGN(4096) : {
        *(.bootstrap_page_tables)
    }

    /* Bootstrap stack section */
    .bootstrap_stack ALIGN(16) : {
        *(.bootstrap_stack)
    }

    /* Store physical end marker for bootstrap section */
    bootstrap_end = .;

    /* Switch to higher half for the rest of the kernel */
    . += KERNEL_VIRTUAL_BASE;

    /* Virtual start marker */
    kernel_virtual_start = .;

    /* Code section */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE) {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE) {
        *(.rodata)
        *(.rodata.*)
    }

    /* Read-write data */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE) {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE) {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note)
    }

    /* End markers */
    kernel_physical_end = . - KERNEL_VIRTUAL_BASE;
    kernel_virtual_end = .;
}
