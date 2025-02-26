#include "gdt.h"
#include <stdio.h>

/* Define entries and pointer for GDT */
struct gdt_entry gdt[5]; // Null, kernel code, kernel data, user code, user data
struct gdt_ptr gp;

/* Define the virtual base address for kernel */
#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000

/* Set up a GDT entry */
void gdt_set_entry(int32_t num, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access = access;
}

/* Set up the GDT */
void setup_gdt() {
    printf("Setting up GDT for 64-bit mode...\n");

    /* Set up GDT pointer */
    gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gp.base = (uint64_t)&gdt;

    /* NULL descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Kernel code segment (64-bit) */
    gdt_set_entry(1, 0, 0xFFFFF, GDT_KERNEL_CODE, 0xAF); // 0xAF = 10101111b = 4KB, 32-bit, long mode

    /* Kernel data segment */
    gdt_set_entry(2, 0, 0xFFFFF, GDT_KERNEL_DATA, 0xCF); // 0xCF = 11001111b = 4KB, 32-bit

    /* User code segment (64-bit) */
    gdt_set_entry(3, 0, 0xFFFFF, GDT_USER_CODE, 0xAF);  // 0xAF = 10101111b = 4KB, 32-bit, long mode

    /* User data segment */
    gdt_set_entry(4, 0, 0xFFFFF, GDT_USER_DATA, 0xCF);  // 0xCF = 11001111b = 4KB, 32-bit

    /* Load the GDT */
    printf("GDT location: %lx\n", (uint64_t)&gp);
    __asm__ __volatile__("lgdt %0" : : "m" (gp));

    /* Reload segment registers */
    __asm__ __volatile__("movq %0, %%rax\n"
                         "movw %%ax, %%ds\n"
                         "movw %%ax, %%es\n"
                         "movw %%ax, %%fs\n"
                         "movw %%ax, %%gs\n"
                         "movw %%ax, %%ss\n"
            : : "r"((uint64_t)GDT_KERNEL_DATA_SEGMENT_SELECTOR));

    /* Far jump to update CS register */
    __asm__ __volatile__("pushq %0\n"
                         "leaq 1f(%%rip), %%rax\n"
                         "pushq %%rax\n"
                         "lretq\n"
                         "1:\n"
            : : "i"((uint64_t)GDT_KERNEL_CODE_SEGMENT_SELECTOR));

    printf("GDT loaded successfully. Running self test...\n");

    /* Verify that segment registers were loaded correctly */
    uint16_t cs, gs, ds, es, fs, ss;
    __asm__ __volatile__("movw %%cs, %0" : "=r"(cs));
    printf("CS selector value: %x, expected value: %x\n", cs, GDT_KERNEL_CODE_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%gs, %0" : "=r"(gs));
    printf("GS selector value: %x, expected value: %x\n", gs, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%ds, %0" : "=r"(ds));
    printf("DS selector value: %x, expected value: %x\n", ds, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%es, %0" : "=r"(es));
    printf("ES selector value: %x, expected value: %x\n", es, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%fs, %0" : "=r"(fs));
    printf("FS selector value: %x, expected value: %x\n", fs, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%ss, %0" : "=r"(ss));
    printf("SS selector value: %x, expected value: %x\n", ss, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    printf("GDT setup and test complete!\n");
}
