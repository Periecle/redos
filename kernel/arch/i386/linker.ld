/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)

/* Define the kernel's virtual base address (3GB mark) */
KERNEL_VIRTUAL_BASE = 0xC0000000;
KERNEL_PHYSICAL_BASE = 0x00100000; /* 1MB, where the bootloader loads us */

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
    /* Begin putting sections at 1 MiB, a conventional place for kernels to be
       loaded at by the bootloader. */
    . = KERNEL_PHYSICAL_BASE;

    /* Physical address of the kernel entry point */
    kernel_physical_start = .;

    /* First put the multiboot header, as it is required to be put very early
       in the image or the bootloader won't recognize the file format.
       Next we'll put the .text section. */
    .bootstrap_text ALIGN(4K) : AT(ADDR(.bootstrap_text))
    {
        /* link the multiboot struct here */
        . = ALIGN(8);
        KEEP(*(.multiboot_header))
        *(.bootstrap_text)
    }

    .bootstrap_data ALIGN(4K) : AT(ADDR(.bootstrap_data))
    {
        *(.bootstrap_data)
    }

    .bootstrap_bss ALIGN(4K) : AT(ADDR(.bootstrap_bss))
    {
        *(.bootstrap_bss)
    }

    . += KERNEL_VIRTUAL_BASE;

    /* The kernel code that runs at the higher half */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE)
    {
        kernel_virtual_start = .;
        *(.text)
    }

    /* Read-only data. */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE)
    {
        *(COMMON)
        *(.bss)
    }

    /* Calculate the physical address where the kernel ends */
    kernel_physical_end = . - KERNEL_VIRTUAL_BASE;
    kernel_virtual_end = .;
}
