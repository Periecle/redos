#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "paging.h"
#include <kernel/tty.h>
#include <kernel/debug.h>
#include <kernel/panic.h>

extern uint64_t kernel_virtual_start;
extern uint64_t kernel_virtual_end;
extern uint64_t kernel_physical_start;
extern uint64_t kernel_physical_end;

void init_paging(void);
void print_paging_info(void);
extern void putchar_enable_serial(bool enable);
extern void printf_enable_serial(bool enable);

#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000

// Write directly to VGA memory for early debugging
void early_print(const char* str) {
    uint16_t* vga = (uint16_t*)0xB8000;
    int pos = 0;

    // Clear screen
    for (int i = 0; i < 80*25; i++) {
        vga[i] = 0x0700; // Black background, white text, space character
    }

    // Print message
    while (*str) {
        vga[pos++] = 0x0F00 | *str++; // White on black
    }
}

/**
 * Kernel main function
 * Entry point after boot sequence completes
 */
void kernel_main(void) {
    // Early debug output directly to VGA
    early_print("Kernel main reached!");

    // For a delay loop to see the message
    for (volatile int i = 0; i < 10000000; i++) {}

    // Initialize TTY for normal output
    terminal_initialize();
    terminal_writestring("Terminal initialized in 64-bit mode\n");

    // Initialize debug subsystem
    debug_init();

    // Enable serial output for terminal and debug
    terminal_enable_serial(true);
    debug_set_level(DEBUG_LEVEL_DEBUG);
    debug_set_target(DEBUG_TARGET_ALL);

    // Welcome messages
    debug_info("RedOS 64-bit kernel starting...");
    terminal_writestring("RedOS 64-bit kernel starting...\n");

    // Display kernel address information
    printf("Kernel Main function at virtual address: 0x%lx\n", (unsigned long)&kernel_main);
    printf("Kernel virtual range: 0x%lx - 0x%lx\n",
           (unsigned long)&kernel_virtual_start,
           (unsigned long)&kernel_virtual_end);
    printf("Kernel physical range: 0x%lx - 0x%lx\n",
           (unsigned long)&kernel_physical_start,
           (unsigned long)&kernel_physical_end);

    // Check if we're running in higher half
    if ((unsigned long)&kernel_main >= KERNEL_VIRTUAL_BASE) {
        printf("Successfully running in higher half!\n");
    } else {
        printf("ERROR: Not running in higher half!\n");
    }

    // Final boot message
    printf("\nRedOS 64-bit successfully booted!\n");
}
