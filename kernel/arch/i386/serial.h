#ifndef ARCH_I386_SERIAL_H
#define ARCH_I386_SERIAL_H

#include <stdint.h>
#include <stdbool.h>

/* Serial port base addresses */
#define SERIAL_COM1_PORT 0x3F8
#define SERIAL_COM2_PORT 0x2F8
#define SERIAL_COM3_PORT 0x3E8
#define SERIAL_COM4_PORT 0x2E8

/* Initialize a serial port with specific settings */
bool serial_init(uint16_t port);

/* Initialize COM1 port */
bool serial_init_com1(void);

/* Check if transmit buffer is empty */
bool serial_is_transmit_ready(uint16_t port);

/* Write a byte to a serial port */
void serial_write_byte(uint16_t port, uint8_t byte);

/* Write a byte to COM1 */
void serial_com1_write_byte(uint8_t byte);

/* Write a string to COM1 */
void serial_com1_write_string(const char* str);

/* Check if receive buffer contains data */
bool serial_is_received(uint16_t port);

/* Read a byte from a serial port */
uint8_t serial_read_byte(uint16_t port);

/* Read a byte from COM1 */
uint8_t serial_com1_read_byte(void);

#endif /* ARCH_I386_SERIAL_H */
