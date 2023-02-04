#include "gdt.h"
#include <stdio.h>

/* Array to hold the GDT entries */
struct gdt_entry gdt[3];
/* GDT pointer to be loaded into the CPU */
struct gdt_ptr gp;

// Function to set a GDT entry
// Reference: Intel Software Developer Manual, Volume 3, Section 3.4.5
void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {

    // Set the lower 16 bits of the base
    gdt[num].base_low = (base & 0xFFFF);

    // Set the next 8 bits of the base
    gdt[num].base_middle = (base >> 16) & 0xFF;

    // Set the upper 8 bits of the base
    gdt[num].base_high = (base >> 24) & 0xFF;

    // Set the lower 16 bits of the limit
    gdt[num].limit_low = (limit & 0xFFFF);

    // Set the lower 4 bits of the granularity byte and
    // the upper 4 bits of the granularity byte from the limit's upper 4 bits
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    // Set the access byte
    gdt[num].access = access;
}


/* Function to install the GDT
 * Entries should be extended later to contain Privilege Level code and data 1(device drivers) and 3 - user level. */
void setup_gdt() {

    // Set the GDT limit to the size of the gdt array - 1
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;

    // Set the GDT base to the address of the gdt array
    gp.base = (uint32_t)&gdt;

    gdt[0].base_low = 0;
    gdt[0].base_middle = 0;
    gdt[0].base_high = 0;
    gdt[0].limit_low = 0;
    gdt[0].access = 0;
    gdt[0].granularity = 0;

    gdt[1].base_low = 0;
    gdt[1].base_middle = 0;
    gdt[1].base_high = 0;
    gdt[1].limit_low = 0xFFFF;
    gdt[1].access = 0x9A;
    gdt[1].granularity = 0xCF;

    gdt[2].base_low = 0;
    gdt[2].base_middle = 0;
    gdt[2].base_high = 0;
    gdt[2].limit_low = 0xFFFF;
    gdt[2].access = 0x92;
    gdt[2].granularity = 0xCF;

    //TODO: investigate what is wrong with gdt_set_entry function and replace lines before that
//    // Set the first GDT entry as the null segment
//    // Reference: Intel Software Developer Manual, Volume 3, Section 3.4.5
//    gdt_set_entry(0, 0, 0, 0, 0);
//
//    // Set the second GDT entry as the code segment for privilege level 0
//    // Reference: Intel Software Developer Manual, Volume 3, Section 3.4.5
//    gdt_set_entry(1, 0, 0xFFFF, GDT_CODE_SEGMENT_PL0, 0xCF);
//
//    // Set the third GDT entry as the data segment for privilege level 0
//    // Reference: Intel Software Developer Manual, Volume 3, Section 3.4.5
//    gdt_set_entry(2, 0, 0xFFFF, GDT_DATA_SEGMENT_PL0, 0xCF);

    /* Load the GDT into the CPU */
    __asm__ __volatile__("lgdt %0" : : "m" (gp));

    /* Reload the segment registers, see Section 3.5 of the SDM */
    __asm__ __volatile__("movl %0, %%eax\n"
                         "movw %%ax, %%ds\n"
                         "movw %%ax, %%es\n"
                         "movw %%ax, %%fs\n"
                         "movw %%ax, %%gs\n"
                         "movw %%ax, %%ss\n"
            : : "r"(GDT_KERNEL_DATA_SEGMENT_SELECTOR));

    /* Jump back to our code */
    __asm__ __volatile__("ljmp %0, $1f\n 1:\n" : : "i"(GDT_KERNEL_CODE_SEGMENT_SELECTOR));

    /* Self test of applying GDT table */

    printf("GDT should be set up. Running self test! \n");

    unsigned short cs, gs, ds, es, fs, ss;
    __asm__ __volatile__("movw %%cs, %0" : "=r"(cs));
    printf("CS selector value: %x, expected value: %x \n", cs, GDT_KERNEL_CODE_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%gs, %0" : "=r"(gs));
    printf("GS selector value: %x, expected value: %x \n", gs, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%ds, %0" : "=r"(ds));
    printf("DS selector value: %x, expected value: %x \n", ds, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%es, %0" : "=r"(es));
    printf("ES selector value: %x, expected value: %x \n", es, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%fs, %0" : "=r"(fs));
    printf("FS selector value: %x, expected value: %x \n", fs, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

    __asm__ __volatile__("movw %%ss, %0" : "=r"(ss));
    printf("SS selector value: %x, expected value: %x \n", ss, GDT_KERNEL_DATA_SEGMENT_SELECTOR);

}