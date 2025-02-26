#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_KERNEL_CODE_SEGMENT_SELECTOR 0x08
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR 0x10

// 64-bit GDT access byte values
#define GDT_PRESENT        0x80
#define GDT_TSS_DESCRIPTOR 0x09
#define GDT_CODE           0x18
#define GDT_DATA           0x10
#define GDT_DPL0           0x00
#define GDT_DPL3           0x60
#define GDT_LONG_MODE      0x20

// Complete access byte values
#define GDT_KERNEL_CODE    (GDT_PRESENT | GDT_CODE | GDT_DPL0 | GDT_LONG_MODE)
#define GDT_KERNEL_DATA    (GDT_PRESENT | GDT_DATA | GDT_DPL0)
#define GDT_USER_CODE      (GDT_PRESENT | GDT_CODE | GDT_DPL3 | GDT_LONG_MODE)
#define GDT_USER_DATA      (GDT_PRESENT | GDT_DATA | GDT_DPL3)

// Structure to represent a GDT entry for 64-bit mode
struct gdt_entry {
    uint16_t limit_low;   // Lower 16 bits of the limit
    uint16_t base_low;    // Lower 16 bits of the base
    uint8_t base_middle;  // Next 8 bits of the base
    uint8_t access;       // Access byte
    uint8_t granularity;  // Granularity byte
    uint8_t base_high;    // Upper 8 bits of the base
} __attribute__((packed));

// Structure for TSS entry (16 bytes for 64-bit)
struct tss_entry {
    uint16_t length;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed));

// Structure to represent the GDT pointer
struct gdt_ptr {
    uint16_t limit;   // Limit of the GDT (size - 1)
    uint64_t base;    // Base address of the GDT
} __attribute__((packed));

#endif // GDT_H
