#ifndef _KERNEL_PANIC_H
#define _KERNEL_PANIC_H

#include <stdint.h>

/**
 * Kernel panic function - prints a message and halts the system
 * @param message The panic message to display
 */
__attribute__((noreturn))
void panic(const char* message);

/**
 * Kernel panic with formatted message
 * @param format Format string (printf style)
 * @param ... Additional arguments for format string
 */
__attribute__((noreturn))
void panicf(const char* format, ...);

/**
 * Dump CPU registers for debugging purposes
 */
void dump_registers(void);

/**
 * Exception handler for CPU exceptions
 * @param exception_number The CPU exception number
 * @param error_code The error code associated with the exception (if any)
 */
__attribute__((noreturn))
void exception_handler(uint32_t exception_number, uint32_t error_code);

#endif /* _KERNEL_PANIC_H */
