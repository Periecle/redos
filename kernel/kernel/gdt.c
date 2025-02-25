#include "gdt.h"
#include <stdio.h>

/* Define entries and pointer for GDT */
struct gdt_entry gdt[3];
struct gdt_ptr gp;

/* Define the virtual base address for kernel */
#define KERNEL_VIRTUAL_BASE 0xC0000000

/* Set up a GDT entry */
void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access = access;
}

/* Set up the GDT */
void setup_gdt() {
    printf("Setting up GDT for higher half kernel...\n");

    /* Set up GDT pointer */
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;

    /* NULL descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Code segment - covers all 4GB */
    gdt_set_entry(1, 0, 0xFFFFFFFF, GDT_CODE_SEGMENT_PL0, 0xCF);

    /* Data segment - covers all 4GB */
    gdt_set_entry(2, 0, 0xFFFFFFFF, GDT_DATA_SEGMENT_PL0, 0xCF);

    /* Load the GDT */
    printf("GDT location: 0x%x\n", (uint32_t)&gp);
    __asm__ __volatile__("lgdt %0" : : "m" (gp));

    /* Reload segment registers */
    __asm__ __volatile__("movl %0, %%eax\n"
                         "movw %%ax, %%ds\n"
                         "movw %%ax, %%es\n"
                         "movw %%ax, %%fs\n"
                         "movw %%ax, %%gs\n"
                         "movw %%ax, %%ss\n"
            : : "r"(GDT_KERNEL_DATA_SEGMENT_SELECTOR));

    /* Far jump to update CS register */
    __asm__ __volatile__("ljmp %0, $1f\n 1:\n" : : "i"(GDT_KERNEL_CODE_SEGMENT_SELECTOR));

    printf("GDT loaded successfully. Running self test...\n");

    /* Verify that segment registers were loaded correctly */
    unsigned short cs, gs, ds, es, fs, ss;
    __asm__ __volatile__("movw %%cs, %0" : "=r"(cs));
    printf("CS selector value: 0x%x, expected value: 0x%x\n", cs, GDT_KERNEL_CODE_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%gs, %0" : "=r"(gs));
    printf("GS selector value: 0x%x, expected value: 0x%x\n", gs, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%ds, %0" : "=r"(ds));
    printf("DS selector value: 0x%x, expected value: 0x%x\n", ds, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%es, %0" : "=r"(es));
    printf("ES selector value: 0x%x, expected value: 0x%x\n", es, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%fs, %0" : "=r"(fs));
    printf("FS selector value: 0x%x, expected value: 0x%x\n", fs, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%ss, %0" : "=r"(ss));
    printf("SS selector value: 0x%x, expected value: 0x%x\n", ss, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    printf("GDT setup and test complete!\n");
}
