/*  kernel.c - the C part of the kernel */
/*  Copyright (C) 1999, 2010  Free Software Foundation, Inc.
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

#include "multiboot2.h"
#include <stdio.h>

#include <kernel/tty.h>


/*  Forward declarations. */
void validate_boot(unsigned long magic, unsigned long addr);

/*  Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void validate_boot(unsigned long magic, unsigned long addr) {
    struct multiboot_tag *tag;
    unsigned size;

    terminal_initialize();

    /*  Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: %x\n", (unsigned) magic);
        return;
    }

    if (addr & 7) {
        printf("Unaligned mbi: %x\n", addr);
        return;
    }

    size = *(unsigned *) addr;
    printf("Announced mbi size %x\n", size);
    for (tag = (struct multiboot_tag *) (addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
                                         + ((tag->size + 7) & ~7))) {
        printf("Tag %x, Size %x\n", tag->type, tag->size);
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                printf("Command line = %s\n",
                       ((struct multiboot_tag_string *) tag)->string);
                break;
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                printf("Boot loader name = %s\n",
                       ((struct multiboot_tag_string *) tag)->string);
                break;
            case MULTIBOOT_TAG_TYPE_MODULE:
                printf("Module at %x-%x. Command line %s\n",
                       ((struct multiboot_tag_module *) tag)->mod_start,
                       ((struct multiboot_tag_module *) tag)->mod_end,
                       ((struct multiboot_tag_module *) tag)->cmdline);
                break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                printf("mem_lower = %dKB, mem_upper = %dKB\n",
                       ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
                       ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
                break;
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                printf("Boot device %x,%d,%d\n",
                       ((struct multiboot_tag_bootdev *) tag)->biosdev,
                       ((struct multiboot_tag_bootdev *) tag)->slice,
                       ((struct multiboot_tag_bootdev *) tag)->part);
                break;
            case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_memory_map_t *mmap;

                printf("mmap\n");

                for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
                     (multiboot_uint8_t *) mmap
                     < (multiboot_uint8_t *) tag + tag->size;
                     mmap = (multiboot_memory_map_t * )
                             ((unsigned long) mmap
                              + ((struct multiboot_tag_mmap *) tag)->entry_size))
                    printf(" base_addr = %x%x,"
                           " length = %x%x, type = %x\n",
                           (unsigned) (mmap->addr >> 32),
                           (unsigned) (mmap->addr & 0xffffffff),
                           (unsigned) (mmap->len >> 32),
                           (unsigned) (mmap->len & 0xffffffff),
                           (unsigned) mmap->type);
            }
                break;

        }
    }
    tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
                                    + ((tag->size + 7) & ~7));
    printf("Total mbi size %x\n", (unsigned) tag - addr);
}
