#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_KERNEL_CODE_SEGMENT_SELECTOR 0x08
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR 0x10

// Segment access byte values for code and data segments
// Reference: Intel Software Developer Manual, Volume 3, Section 3.4.5
#define GDT_CODE_SEGMENT 0x0A
#define GDT_DATA_SEGMENT 0x02

// Base values for segment access bytes (without DPL bits)
#define GDT_BASE_CODE_SEGMENT 0x9A  // 1001 1010b (Code Segment, Executable, Readable)
#define GDT_BASE_DATA_SEGMENT 0x92  // 1001 0010b (Data Segment, Writable)

// Privilege level bits
#define GDT_DPL0 0x00  // Descriptor Privilege Level 0 (Kernel)
#define GDT_DPL1 0x20  // Descriptor Privilege Level 1
#define GDT_DPL2 0x40  // Descriptor Privilege Level 2
#define GDT_DPL3 0x60  // Descriptor Privilege Level 3 (User)

// Access byte values for privilege level 0 and 3
// Reference: Intel Software Developer Manual, Volume 3, Section 3.5
// Code segment access byte values for privilege levels 0-3
#define GDT_CODE_SEGMENT_PL0 (GDT_BASE_CODE_SEGMENT | GDT_DPL0)
#define GDT_CODE_SEGMENT_PL1 (GDT_BASE_CODE_SEGMENT | GDT_DPL1)
#define GDT_CODE_SEGMENT_PL2 (GDT_BASE_CODE_SEGMENT | GDT_DPL2)
#define GDT_CODE_SEGMENT_PL3 (GDT_BASE_CODE_SEGMENT | GDT_DPL3)

// Data segment access byte values for different privilege levels 0-3
#define GDT_DATA_SEGMENT_PL0 (GDT_BASE_DATA_SEGMENT | GDT_DPL0)
#define GDT_DATA_SEGMENT_PL1 (GDT_BASE_DATA_SEGMENT | GDT_DPL1)
#define GDT_DATA_SEGMENT_PL2 (GDT_BASE_DATA_SEGMENT | GDT_DPL2)
#define GDT_DATA_SEGMENT_PL3 (GDT_BASE_DATA_SEGMENT | GDT_DPL3)

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

