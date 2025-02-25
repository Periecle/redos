#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "paging.h"
#include <kernel/tty.h>
#include <kernel/debug.h>
#include <kernel/panic.h>

extern uint32_t kernel_virtual_start;
extern uint32_t kernel_virtual_end;
extern uint32_t kernel_physical_start;
extern uint32_t kernel_physical_end;

/* Function declarations */
void init_paging(void);
void print_paging_info(void);
extern void putchar_enable_serial(bool enable);
extern void printf_enable_serial(bool enable);

#define KERNEL_VIRTUAL_BASE 0xC0000000

static inline int is_higher_half_address(uint32_t addr) {
    return addr >= KERNEL_VIRTUAL_BASE;
}

void print_kernel_memory_layout(void) {
    printf("Kernel Memory Layout:\n");
    printf("  Virtual Start:  0x%x\n", &kernel_virtual_start);
    printf("  Virtual End:    0x%x\n", &kernel_virtual_end);
    printf("  Physical Start: 0x%x\n", &kernel_physical_start);
    printf("  Physical End:   0x%x\n", &kernel_physical_end);
    printf("  Virtual Size:   %d KB\n",
        (&kernel_virtual_end - &kernel_virtual_start) / 1024);
}

void test_debugging_levels(void) {
    debug_error("This is an ERROR level message");
    debug_warning("This is a WARNING level message");
    debug_info("This is an INFO level message");
    debug_debug("This is a DEBUG level message");
    debug_trace("This is a TRACE level message");

    printf("\nTesting printf redirection to serial port\n");
}

void kernel_main(void) {
    /* Initialize debug subsystem (including serial port) */
    debug_init();

    /* Configure serial output - EITHER enable terminal serial OR printf serial, not both */
    terminal_enable_serial(true);      /* Let terminal functions handle serial output */
    printf_enable_serial(false);       /* Disable printf direct serial output since terminal handles it */
    putchar_enable_serial(false);      /* Disable putchar direct serial output since terminal handles it */


    /* Set debug level and target (both VGA and Serial)
    * Terminal will handle actual serial output */
    debug_set_level(DEBUG_LEVEL_DEBUG);
    debug_set_target(DEBUG_TARGET_ALL);  /* This is fine as debug_write now checks terminal_is_serial_enabled */

    debug_info("RedOS kernel starting...");
    debug_info("Serial debugging enabled");

    /* Test different debug levels */
    test_debugging_levels();

    printf("Kernel Main function called at virtual address 0x%x!\n", (uint32_t)&kernel_main);

    if (is_higher_half_address((uint32_t)&kernel_main)) {
        debug_info("Kernel is running in the higher half (0xC0000000+)");
    } else {
        debug_error("Kernel is NOT running in the higher half!");
    }

    print_kernel_memory_layout();
    init_paging();
    print_paging_info();

    debug_info("Testing physical memory allocation");
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

    debug_info("Testing virtual memory mapping");
    void* test_virt_addr = (void*)0xD0000000;
    printf("\nTesting virtual memory mapping:\n");
    printf("  Mapping virtual 0x%x to physical 0x%x\n",
        (uint32_t)test_virt_addr, (uint32_t)page1);
    map_page_to_frame(test_virt_addr, page1, PAGE_PRESENT | PAGE_WRITE);

    debug_debug("Writing test pattern to mapped memory");
    printf("  Writing test pattern to mapped memory...\n");
    uint32_t* test_ptr = (uint32_t*)test_virt_addr;
    *test_ptr = 0xDEADBEEF;
    printf("  Reading back value: 0x%x (expected 0xDEADBEEF)\n", *test_ptr);

    debug_debug("Cleaning up test allocations");
    printf("\nCleaning up test allocations:\n");
    printf("  Unmapping virtual address 0x%x\n", (uint32_t)test_virt_addr);
    unmap_page(test_virt_addr);
    printf("  Freeing physical pages\n");
    kfree_physical_page(page1);
    kfree_physical_page(page2);
    kfree_physical_page(page3);

    print_paging_info();

    /* Show how to use memory dump feature */
    debug_info("Memory dump of kernel start area");
    debug_hex_dump(&kernel_virtual_start, 128);

    /* Test the different output targets */
    debug_info("Testing debug output targets");

    debug_set_target(DEBUG_TARGET_VGA);
    debug_info("This message should only appear on VGA (not in serial log)");

    debug_set_target(DEBUG_TARGET_SERIAL);
    debug_info("This message should only appear in serial log (not on VGA)");

    debug_set_target(DEBUG_TARGET_ALL);
    debug_info("This message should appear in both VGA and serial log");

    /* Test panic function (uncomment to test) */
    // panic("Test panic message!");

    debug_info("RedOS successfully booted in higher half mode!");
    printf("\nRedOS successfully booted in higher half mode!\n");
}
