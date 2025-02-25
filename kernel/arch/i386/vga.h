#ifndef ARCH_I386_VGA_H
#define ARCH_I386_VGA_H

#include <stdint.h>

/* Define the virtual base address for kernel */
#define KERNEL_VIRTUAL_BASE 0xC0000000

/* VGA buffer physical address */
#define VGA_BUFFER_PHYSICAL 0xB8000

/* VGA buffer virtual address (used after paging is enabled) */
#define VGA_BUFFER_VIRTUAL (VGA_BUFFER_PHYSICAL + KERNEL_VIRTUAL_BASE)

/* Determine if paging is enabled by checking CR0 register */
static inline int is_paging_enabled(void) {
    uint32_t cr0;
    __asm__ volatile("movl %%cr0, %0" : "=r"(cr0));
    return (cr0 & 0x80000000) != 0; /* Check PG bit */
}

/* Get the appropriate VGA buffer address based on paging state */
static inline uint16_t* get_vga_buffer(void) {
    if (is_paging_enabled()) {
        return (uint16_t*)VGA_BUFFER_VIRTUAL;
    } else {
        return (uint16_t*)VGA_BUFFER_PHYSICAL;
    }
}

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

#endif
