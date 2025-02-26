#include <stdint.h>
#include <stdbool.h>
#include "serial.h"

/* I/O port addresses for COM1 */
#define COM1_PORT 0x3F8

/* COM port registers (offset from base port) */
#define REG_DATA        0 /* Data register (R/W) */
#define REG_INT_ENABLE  1 /* Interrupt enable (W) */
#define REG_INT_ID      2 /* Interrupt identification / FIFO control (R/W) */
#define REG_LINE_CTRL   3 /* Line control register (W) */
#define REG_MODEM_CTRL  4 /* Modem control register (W) */
#define REG_LINE_STATUS 5 /* Line status register (R) */
#define REG_MODEM_STATUS 6 /* Modem status register (R) */
#define REG_SCRATCH     7 /* Scratch register (R/W) */

/* Line status register bits */
#define LSR_DATA_READY  0x01 /* Data ready */
#define LSR_TX_EMPTY    0x20 /* Transmitter holding register empty */

/* CPU I/O functions */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

bool serial_init(uint16_t port) {
    /* Disable interrupts */
    outb(port + REG_INT_ENABLE, 0x00);

    /* Set baud rate - 115200 baud (divisor = 1) */
    outb(port + REG_LINE_CTRL, 0x80);    /* Enable DLAB */
    outb(port + REG_DATA, 0x01);         /* Set divisor low byte */
    outb(port + REG_INT_ENABLE, 0x00);   /* Set divisor high byte */

    /* 8 bits, 1 stop bit, no parity */
    outb(port + REG_LINE_CTRL, 0x03);   // 8N1, no echo

    /* Enable FIFO, clear them, with 14-byte threshold */
    outb(port + REG_INT_ID, 0xC7);

    /* Enable IRQs, set RTS/DSR, DISABLE echo */
    outb(port + REG_MODEM_CTRL, 0x0B);

    /* Test the serial chip (still need a loopback test) */
    outb(port + REG_MODEM_CTRL, 0x1E);   /* Set loopback mode temporarily */
    outb(port + REG_DATA, 0xAE);         /* Send test byte */

    /* Check if the same byte is received */
    if (inb(port + REG_DATA) != 0xAE) {
        return false;
    }

    /* Disable loopback mode, back to normal operation WITHOUT echo */
    outb(port + REG_MODEM_CTRL, 0x0F);

    return true;
}

/* Initialize COM1 port */
bool serial_init_com1(void) {
    return serial_init(COM1_PORT);
}

/* Check if transmit is empty */
bool serial_is_transmit_ready(uint16_t port) {
    return (inb(port + REG_LINE_STATUS) & LSR_TX_EMPTY) != 0;
}

/* Send a byte to the specified serial port */
void serial_write_byte(uint16_t port, uint8_t byte) {
    /* Wait until transmit is ready */
    while (!serial_is_transmit_ready(port)) {
        /* Busy wait */
    }

    /* Send the byte */
    outb(port + REG_DATA, byte);
}

/* Send a byte to COM1 */
void serial_com1_write_byte(uint8_t byte) {
    serial_write_byte(COM1_PORT, byte);
}

/* Write a string to COM1 */
void serial_com1_write_string(const char* str) {
    while (*str != '\0') {
        serial_com1_write_byte(*str++);
    }
}

/* Check if receive contains data */
bool serial_is_received(uint16_t port) {
    return (inb(port + REG_LINE_STATUS) & LSR_DATA_READY) != 0;
}

/* Read a byte from the specified serial port */
uint8_t serial_read_byte(uint16_t port) {
    /* Wait until data is available */
    while (!serial_is_received(port)) {
        /* Busy wait */
    }

    /* Read the byte */
    return inb(port + REG_DATA);
}

/* Read a byte from COM1 */
uint8_t serial_com1_read_byte(void) {
    return serial_read_byte(COM1_PORT);
}
