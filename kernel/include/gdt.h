#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_KERNEL_CODE_SEGMENT_SELECTOR 0x08
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR 0x10

// Segment access byte values for code and data segments
// Reference: Intel Software Developer Manual, Volume 3, Section 3.4.5
#define GDT_CODE_SEGMENT 0x0A
#define GDT_DATA_SEGMENT 0x02

// Access byte values for privilege level 0 and 3
// Reference: Intel Software Developer Manual, Volume 3, Section 3.5
#define GDT_CODE_SEGMENT_PL0 (GDT_CODE_SEGMENT | 0x00)
#define GDT_CODE_SEGMENT_PL3 (GDT_CODE_SEGMENT | 0x60)
#define GDT_DATA_SEGMENT_PL0 (GDT_DATA_SEGMENT | 0x00)
#define GDT_DATA_SEGMENT_PL3 (GDT_DATA_SEGMENT | 0x60)

// Structure to represent a GDT entry
// Reference: Intel Software Developer Manual, Volume 3, Section 3.4
struct gdt_entry {
    uint16_t limit_low;   // Lower 16 bits of the limit
    uint16_t base_low;    // Lower 16 bits of the base
    uint8_t base_middle;  // Next 8 bits of the base
    uint8_t access;       // Access byte
    uint8_t granularity;  // Granularity byte
    uint8_t base_high;    // Upper 8 bits of the base
} __attribute__((packed));

// Structure to represent the GDT pointer
// Reference: Intel Software Developer Manual, Volume 3, Section 3.4
struct gdt_ptr {
    uint16_t limit;   // Limit of the GDT
    uint32_t base;    // Base address of the GDT
} __attribute__((packed));


#endif // GDT_H

