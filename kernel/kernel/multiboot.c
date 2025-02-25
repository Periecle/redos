#include "multiboot2.h"
#include <stdio.h>
#include <stdint.h>
#include <kernel/tty.h>

/* Define the virtual base address for kernel */
#define KERNEL_VIRTUAL_BASE 0xC0000000

/* Function to map a physical address to virtual address */
static inline void* P2V(void* addr) {
    return (void*)((uint32_t) addr + KERNEL_VIRTUAL_BASE);
}

/* Function to validate multiboot information */
void validate_boot(unsigned long magic, unsigned long addr) {
    struct multiboot_tag *tag;
    unsigned size;

    printf("Validating multiboot information at virtual address %x\n", addr);

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: %x (expected %x)\n",
            (unsigned)magic, MULTIBOOT2_BOOTLOADER_MAGIC);
        return;
    }

    if (addr & 7) {
        printf("Unaligned multiboot info pointer: %x\n", addr);
        return;
    }

    /* Convert physical address to virtual if needed */
    if (addr < KERNEL_VIRTUAL_BASE) {
        printf("Multiboot info pointer appears to be a physical address, converting...\n");
        addr += KERNEL_VIRTUAL_BASE;
        printf("Converted to virtual address: %x\n", addr);
    }

    size = *(unsigned*)addr;
    printf("Multiboot info size: %d bytes\n", size);

    for (tag = (struct multiboot_tag*)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7))) {

        printf("Tag %x, Size %x\n", tag->type, tag->size);

        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                printf("Command line = %s\n",
                       ((struct multiboot_tag_string*)tag)->string);
                break;

            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                printf("Boot loader name = %s\n",
                       ((struct multiboot_tag_string*)tag)->string);
                break;

            case MULTIBOOT_TAG_TYPE_MODULE:
                printf("Module at %x-%x. Command line %s\n",
                       ((struct multiboot_tag_module*)tag)->mod_start,
                       ((struct multiboot_tag_module*)tag)->mod_end,
                       ((struct multiboot_tag_module*)tag)->cmdline);
                break;

            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                printf("mem_lower = %dKB, mem_upper = %dKB\n",
                       ((struct multiboot_tag_basic_meminfo*)tag)->mem_lower,
                       ((struct multiboot_tag_basic_meminfo*)tag)->mem_upper);
                break;

            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                printf("Boot device %x,%d,%d\n",
                       ((struct multiboot_tag_bootdev*)tag)->biosdev,
                       ((struct multiboot_tag_bootdev*)tag)->slice,
                       ((struct multiboot_tag_bootdev*)tag)->part);
                break;

            case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_memory_map_t *mmap;
                printf("Memory map:\n");

                for (mmap = ((struct multiboot_tag_mmap*)tag)->entries;
                     (multiboot_uint8_t*)mmap < (multiboot_uint8_t*)tag + tag->size;
                     mmap = (multiboot_memory_map_t*)((unsigned long)mmap + ((struct multiboot_tag_mmap*)tag)->entry_size)) {

                    printf("  Region: base=%x%08x, length=%x%08x, type=%d\n",
                           (unsigned)(mmap->addr >> 32),
                           (unsigned)(mmap->addr & 0xffffffff),
                           (unsigned)(mmap->len >> 32),
                           (unsigned)(mmap->len & 0xffffffff),
                           (unsigned)mmap->type);
                }
                break;
            }
        }
    }

    tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7));
    printf("Total multiboot info size: %d bytes\n", (unsigned)tag - addr);
    printf("Multiboot validation complete!\n");
}
