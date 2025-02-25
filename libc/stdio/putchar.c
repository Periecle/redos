#include <stdio.h>
#include <stdbool.h>

#if defined(__is_libc)
#include "../../kernel/include/kernel/tty.h"
#endif

/* Flag to control if putchar should also output to serial port */
static bool putchar_serial_output = false;

/* Function declarations for serial port (implemented elsewhere) */
extern void serial_com1_write_byte(unsigned char byte);
extern bool terminal_is_serial_enabled(void);

/* Enable/disable serial output for putchar */
void putchar_enable_serial(bool enable) {
    putchar_serial_output = enable;
}

int putchar(int ic) {
    char c = (char) ic;

#if defined(__is_libc)
    /* Output to terminal (which may handle serial output itself) */
    terminal_write(&c, sizeof(c));

    /* Output to serial if enabled AND terminal isn't already doing it */
    if (putchar_serial_output && !terminal_is_serial_enabled()) {
        /* Add CR before LF for serial terminals */
        if (c == '\n') {
            serial_com1_write_byte('\r');
        }
        serial_com1_write_byte((unsigned char)c);
    }
#else
    /* Non-kernel implementation would go here */
#endif

    return ic;
}
