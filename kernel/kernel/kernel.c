#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "paging.h"

/* Forward declarations */
extern uint32_t kernel_virtual_start;
extern uint32_t kernel_virtual_end;
extern uint32_t kernel_physical_start;
extern uint32_t kernel_physical_end;

/* Forward declaration for initialization functions */
void init_paging(void);
void print_paging_info(void);

/* Define the virtual base address for kernel */
#define KERNEL_VIRTUAL_BASE 0xC0000000

/* Determine if an address is mapped in the higher half */
static inline int is_higher_half_address(uint32_t addr) {
    return addr >= KERNEL_VIRTUAL_BASE;
}

/* Print kernel memory layout information */
void print_kernel_memory_layout(void) {
    printf("Kernel Memory Layout:\n");
    printf("  Virtual Start:  0x%x\n", &kernel_virtual_start);
    printf("  Virtual End:    0x%x\n", &kernel_virtual_end);
    printf("  Physical Start: 0x%x\n", &kernel_physical_start);
    printf("  Physical End:   0x%x\n", &kernel_physical_end);
    printf("  Virtual Size:   %d KB\n",
        (&kernel_virtual_end - &kernel_virtual_start) / 1024);
}

/* Main kernel entry point */
void kernel_main(void) {
    printf("Kernel Main function called at virtual address 0x%x!\n", (uint32_t)&kernel_main);

    /* Check if we're really running in the higher half */
    if (is_higher_half_address((uint32_t)&kernel_main)) {
        printf("SUCCESS: Kernel is running in the higher half (0xC0000000+)\n");
    } else {
        printf("ERROR: Kernel is NOT running in the higher half!\n");
    }

    /* Print memory layout information */
    print_kernel_memory_layout();

    /* Initialize our paging system fully */
    init_paging();

    /* Print detailed paging information */
    print_paging_info();

    /* Test memory allocation */
    printf("\nTesting physical memory allocation:\n");
    void* page1 = kmalloc_physical_page();
    void* page2 = kmalloc_physical_page();
    void* page3 = kmalloc_physical_page();

    printf("  Allocated page 1 at physical: 0x%x, virtual: 0x%x\n",
        (uint32_t)page1, (uint32_t)P2V(page1));
    printf("  Allocated page 2 at physical: 0x%x, virtual: 0x%x\n",
        (uint32_t)page2, (uint32_t)P2V(page2));
    printf("  Allocated page 3 at physical: 0x%x, virtual: 0x%x\n",
        (uint32_t)page3, (uint32_t)P2V(page3));

    /* Map a new virtual page to test our paging functions */
    void* test_virt_addr = (void*)0xD0000000; /* Choose an unused virtual address */
    printf("\nTesting virtual memory mapping:\n");
    printf("  Mapping virtual 0x%x to physical 0x%x\n",
        (uint32_t)test_virt_addr, (uint32_t)page1);

    map_page_to_frame(test_virt_addr, page1, PAGE_PRESENT | PAGE_WRITE);

    /* Try writing to the mapped memory */
    printf("  Writing test pattern to mapped memory...\n");
    uint32_t* test_ptr = (uint32_t*)test_virt_addr;
    *test_ptr = 0xDEADBEEF;

    /* Verify we can read it back */
    printf("  Reading back value: 0x%x (expected 0xDEADBEEF)\n", *test_ptr);

    /* Free resources */
    printf("\nCleaning up test allocations:\n");
    printf("  Unmapping virtual address 0x%x\n", (uint32_t)test_virt_addr);
    unmap_page(test_virt_addr);

    printf("  Freeing physical pages\n");
    kfree_physical_page(page1);
    kfree_physical_page(page2);
    kfree_physical_page(page3);

    /* Print paging info again to show changes */
    print_paging_info();

    printf("\nRedos OS successfully booted in higher half mode!\n");
}
