#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/tty.h>
#include "serial.h"
#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t *terminal_buffer;

static bool terminal_serial_output = false;

void terminal_enable_serial(bool enable) {
    terminal_serial_output = enable;
}

bool terminal_is_serial_enabled(void) {
    return terminal_serial_output;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    // Use our helper function to get the right VGA buffer address
    terminal_buffer = get_vga_buffer();

    // Print initialization message to debug
    const char* paging_status = is_paging_enabled() ? "ENABLED" : "DISABLED";

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }

    // Output some debug info about our terminal initialization
    terminal_writestring("Terminal initialized with paging ");
    terminal_writestring(paging_status);
    terminal_writestring("\nVGA buffer address: ");

    // Print the address in hex (simple implementation)
    char hex_buffer[20] = "0x";
    uint32_t addr = (uint32_t)terminal_buffer;

    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (addr >> (i * 4)) & 0xF;
        hex_buffer[9-i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
    }
    hex_buffer[10] = '\0';

    terminal_writestring(hex_buffer);
    terminal_writestring("\n");
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void scroll(void) {
    memmove(terminal_buffer,
            terminal_buffer + VGA_WIDTH,
            sizeof(uint16_t) * VGA_WIDTH * (VGA_HEIGHT - 1));
    memset(terminal_buffer + VGA_WIDTH * (VGA_HEIGHT - 1),
           0,
           sizeof(uint16_t) * VGA_WIDTH);
    if (terminal_row > 0) {
        terminal_row--;
    }
}

void terminal_putchar(char c) {
    /* Output to VGA */
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            scroll();
        }

        /* Output to serial port if enabled */
        if (terminal_serial_output) {
            serial_com1_write_byte('\r');
            serial_com1_write_byte('\n');
        }
        return;
    }

    if (c == '\r') {
        terminal_column = 0;

        /* Output to serial port if enabled */
        if (terminal_serial_output) {
            serial_com1_write_byte('\r');
        }
        return;
    }

    if (c == '\t') {
        terminal_column += 4;
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
            if (terminal_row >= VGA_HEIGHT) {
                scroll();
            }
        }

        /* Output to serial port if enabled */
        if (terminal_serial_output) {
            serial_com1_write_byte('\t');
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            scroll();
        }
    }

    /* Output to serial port if enabled */
    if (terminal_serial_output) {
        serial_com1_write_byte(c);
    }
}

void terminal_write(const char *data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char *data) {
    terminal_write(data, strlen(data));
}
