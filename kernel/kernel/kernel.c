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

void init_paging(void);
void print_paging_info(void);
extern void putchar_enable_serial(bool enable);
extern void printf_enable_serial(bool enable);

#define KERNEL_VIRTUAL_BASE 0xC0000000

/**
 * Check if an address is in the higher half of memory (kernel space)
 * @param addr Address to check
 * @return true if address is in higher half, false otherwise
 */
static inline int is_higher_half_address(uint32_t addr) {
    return addr >= KERNEL_VIRTUAL_BASE;
}

/**
 * Print kernel memory layout information
 */
void print_kernel_memory_layout(void) {
    printf("Kernel Memory Layout:\n");
    printf("  Virtual Start:  %x\n", (unsigned)&kernel_virtual_start);
    printf("  Virtual End:    %x\n", (unsigned)&kernel_virtual_end);
    printf("  Physical Start: %x\n", (unsigned)&kernel_physical_start);
    printf("  Physical End:   %x\n", (unsigned)&kernel_physical_end);
    printf("  Virtual Size:   %u KB\n",
        (unsigned)(&kernel_virtual_end - &kernel_virtual_start) / 1024);
}

/**
 * Test different debug output levels
 */
void test_debugging_levels(void) {
    debug_error("This is an ERROR level message");
    debug_warning("This is a WARNING level message");
    debug_info("This is an INFO level message");
    debug_debug("This is a DEBUG level message");
    debug_trace("This is a TRACE level message");
    printf("\nTesting printf redirection to serial port\n");
}

/**
 * Test memory mapping functionality
 * Maps a test value and confirms it can be read back correctly
 */
void test_memory_mapping(void) {
    debug_info("Testing virtual memory mapping");
    printf("\nTesting virtual memory mapping:\n");

    // Allocate physical pages
    void* page1 = kmalloc_physical_page();
    void* page2 = kmalloc_physical_page();
    void* page3 = kmalloc_physical_page();

    printf("  Allocated page 1 at physical: %x, virtual: %x\n",
        (unsigned)page1, (unsigned)P2V(page1));
    printf("  Allocated page 2 at physical: %x, virtual: %x\n",
        (unsigned)page2, (unsigned)P2V(page2));
    printf("  Allocated page 3 at physical: %x, virtual: %x\n",
        (unsigned)page3, (unsigned)P2V(page3));

    // Choose a test virtual address in user space (not kernel)
    void* test_virt_addr = (void*)0xD0000000;
    printf("  Mapping virtual %x to physical %x\n",
        (unsigned)test_virt_addr, (unsigned)page1);

    // Map the virtual address to the physical page
    map_page_to_frame(test_virt_addr, page1, PAGE_PRESENT | PAGE_WRITE);

    // Test writing to and reading from the mapped memory
    debug_debug("Writing test pattern to mapped memory");
    printf("  Writing test pattern to mapped memory...\n");

    // Write a known pattern
    uint32_t* test_ptr = (uint32_t*)test_virt_addr;
    *test_ptr = 0xDEADBEEF;

    // Flush the TLB to ensure the mapping is active
    flush_tlb_entry((uint32_t)test_virt_addr);

    // Read back and verify
    uint32_t read_value = *test_ptr;
    printf("  Reading back value: %x (expected 0xDEADBEEF)\n", read_value);

    if (read_value != 0xDEADBEEF) {
        debug_error("Memory test failed! Expected 0xDEADBEEF, got %x", read_value);
    } else {
        debug_info("Memory test passed successfully");
    }

    // Clean up
    debug_debug("Cleaning up test allocations");
    printf("\nCleaning up test allocations:\n");
    printf("  Unmapping virtual address %x\n", (unsigned)test_virt_addr);
    unmap_page(test_virt_addr);

    printf("  Freeing physical pages\n");
    kfree_physical_page(page1);
    kfree_physical_page(page2);
    kfree_physical_page(page3);
}

/**
 * Kernel main function
 * Entry point after boot sequence completes
 */
void kernel_main(void) {
    // Initialize debug subsystem first for logging
    debug_init();

    // Enable serial output for terminal but not for printf/putchar
    terminal_enable_serial(true);
    printf_enable_serial(false);
    putchar_enable_serial(false);

    // Set debug level and output targets
    debug_set_level(DEBUG_LEVEL_DEBUG);
    debug_set_target(DEBUG_TARGET_ALL);

    // Welcome messages
    debug_info("RedOS kernel starting...");
    debug_info("Serial debugging enabled");

    // Test the debug output system
    test_debugging_levels();

    // Display kernel main address
    printf("Kernel Main function called at virtual address %x!\n", (unsigned)&kernel_main);

    // Check if kernel is running in higher half
    if (is_higher_half_address((unsigned)&kernel_main)) {
        debug_info("Kernel is running in the higher half (0xC0000000+)");
    } else {
        debug_error("Kernel is NOT running in the higher half!");
        panic("Kernel not in higher half");
    }

    // Display memory layout
    print_kernel_memory_layout();

    // Initialize paging (if not already done by boot)
    init_paging();
    print_paging_info();

    // Test memory allocation and mapping
    test_memory_mapping();

    // Display updated paging info
    print_paging_info();

    // Test memory dump functionality
    debug_info("Memory dump of kernel start area");
    debug_hex_dump(&kernel_virtual_start, 128);

    // Test debug output target switching
    debug_info("Testing debug output targets");
    debug_set_target(DEBUG_TARGET_VGA);
    debug_info("This message should only appear on VGA (not in serial log)");
    debug_set_target(DEBUG_TARGET_SERIAL);
    debug_info("This message should only appear in serial log (not on VGA)");
    debug_set_target(DEBUG_TARGET_ALL);
    debug_info("This message should appear in both VGA and serial log");

    // Final boot success message
    debug_info("RedOS successfully booted in higher half mode!");
    printf("\nRedOS successfully booted in higher half mode!\n");
}
