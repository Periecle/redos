#include "multiboot2.h"
#include <stdio.h>
#include <stdint.h>
#include <kernel/tty.h>

/* Define the virtual base address for kernel */
#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000

/* Function to map a physical address to virtual address */
static inline void* P2V(void* addr) {
    if ((uint64_t)addr >= KERNEL_VIRTUAL_BASE) {
        return addr; // Already a virtual address
    }
    return (void*)((uint64_t)addr + KERNEL_VIRTUAL_BASE);
}

/* Function to validate multiboot information */
void validate_boot(unsigned long magic, unsigned long addr) {
    struct multiboot_tag *tag;
    unsigned size;

    printf("Validating multiboot information:\n");
    printf("  Magic: 0x%lx, Info: 0x%lx\n", magic, addr);

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%lx (expected 0x%lx)\n",
            magic, (unsigned long)MULTIBOOT2_BOOTLOADER_MAGIC);
        return;
    }

    /* Convert physical address to virtual if needed */
    addr = (unsigned long)P2V((void*)addr);
    printf("Using address: 0x%lx\n", addr);

    size = *(unsigned*)addr;
    printf("Multiboot info size: %d bytes\n", size);

    for (tag = (struct multiboot_tag*)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7))) {

        printf("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);

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
                printf("Module at 0x%lx-0x%lx. Command line %s\n",
                       (unsigned long)((struct multiboot_tag_module*)tag)->mod_start,
                       (unsigned long)((struct multiboot_tag_module*)tag)->mod_end,
                       ((struct multiboot_tag_module*)tag)->cmdline);
                break;

            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                printf("mem_lower = %uKB, mem_upper = %uKB\n",
                       ((struct multiboot_tag_basic_meminfo*)tag)->mem_lower,
                       ((struct multiboot_tag_basic_meminfo*)tag)->mem_upper);
                break;

            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                printf("Boot device 0x%x,%u,%u\n",
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

                    printf("  Region: base=0x%llx, length=0x%llx, type=%u\n",
                           mmap->addr,
                           mmap->len,
                           (unsigned)mmap->type);
                }
                break;
            }
        }
    }

    tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7));
    printf("Total multiboot info size: %lu bytes\n", (unsigned long)tag - addr);
    printf("Multiboot validation complete!\n");
}
