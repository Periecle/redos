#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/debug.h>
#include "../arch/i386/serial.h"

/* Current debug settings */
static int debug_level = DEBUG_LEVEL_INFO;
static int debug_target = DEBUG_TARGET_VGA;
static bool debug_initialized = false;

/* Buffer for formatting debug output */
#define DEBUG_BUFFER_SIZE 1024
static char debug_buffer[DEBUG_BUFFER_SIZE];

/* Level prefixes */
static const char* level_prefix[] = {
    "",         /* NONE */
    "[ERROR] ", /* ERROR */
    "[WARN]  ", /* WARNING */
    "[INFO]  ", /* INFO */
    "[DEBUG] ", /* DEBUG */
    "[TRACE] ", /* TRACE */
};

/* Initialize the debug subsystem */
void debug_init(void) {
    if (debug_initialized) {
        return;
    }

    /* Initialize serial port */
    bool serial_ok = serial_init_com1();

    /* Set default debug level and target */
    debug_level = DEBUG_LEVEL_INFO;
    debug_target = DEBUG_TARGET_VGA;

    /* Add serial target if it was initialized successfully */
    if (serial_ok) {
        debug_target |= DEBUG_TARGET_SERIAL;
    }

    debug_initialized = true;

    /* Initial debug message */
    debug_info("Debug subsystem initialized");
    if (serial_ok) {
        debug_info("Serial COM1 port initialized");
    } else {
        debug_warning("Failed to initialize serial COM1 port");
    }
}

/* Set the current debug level */
void debug_set_level(int level) {
    if (level >= DEBUG_LEVEL_NONE && level <= DEBUG_LEVEL_TRACE) {
        debug_level = level;
    }
}

/* Get the current debug level */
int debug_get_level(void) {
    return debug_level;
}

/* Set the debug output target */
void debug_set_target(int target) {
    debug_target = target;
}

/* Get the current debug output target */
int debug_get_target(void) {
    return debug_target;
}

/* Write a formatted string to the specified debug target */
static void debug_write(const char* str) {
    /* Check if we should write to VGA */
    if (debug_target & DEBUG_TARGET_VGA) {
        terminal_writestring(str);
    }

    /* Check if we should write directly to serial
     * ONLY do this if terminal is NOT already handling serial */
    if ((debug_target & DEBUG_TARGET_SERIAL) && !terminal_is_serial_enabled()) {
        serial_com1_write_string(str);
    }
}

/* Custom implementation of vsnprintf for our limited environment */
int vsnprintf(char* str, size_t size, const char* format, va_list args) {
    /* This is a very simplified implementation that just calls printf and captures its output */
    /* In a real implementation, we would reimplement the formatting logic completely */
    /* For now, we'll format into a temporary buffer and then copy to the destination */
    char temp_buffer[DEBUG_BUFFER_SIZE];
    int i = 0;

    /* Format the string */
    int written = 0;
    while (format[i] && i < DEBUG_BUFFER_SIZE - 1) {
        if (format[i] == '%' && format[i+1] != '\0') {
            i++;
            switch (format[i]) {
                case 'd': {
                    int val = va_arg(args, int);
                    /* Simple conversion for integers */
                    char num_buffer[32];
                    int num_pos = 0;
                    int temp_val = val;

                    /* Handle negative numbers */
                    if (val < 0) {
                        temp_buffer[written++] = '-';
                        temp_val = -val;
                    }

                    /* Handle zero case */
                    if (temp_val == 0) {
                        num_buffer[num_pos++] = '0';
                    }

                    /* Convert to string in reverse order */
                    while (temp_val > 0) {
                        num_buffer[num_pos++] = '0' + (temp_val % 10);
                        temp_val /= 10;
                    }

                    /* Copy to output buffer in correct order */
                    while (num_pos > 0) {
                        temp_buffer[written++] = num_buffer[--num_pos];
                    }
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    while (*s && written < DEBUG_BUFFER_SIZE - 1) {
                        temp_buffer[written++] = *s++;
                    }
                    break;
                }
                case 'x': {
                    int val = va_arg(args, int);
                    /* Format as hex */
                    temp_buffer[written++] = '0';
                    temp_buffer[written++] = 'x';

                    /* Handle zero case */
                    if (val == 0) {
                        temp_buffer[written++] = '0';
                        break;
                    }

                    /* Convert to hex digits */
                    char hex_buffer[32];
                    int hex_pos = 0;
                    while (val > 0 && hex_pos < 32) {
                        int digit = val & 0xF;
                        hex_buffer[hex_pos++] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
                        val >>= 4;
                    }

                    /* Copy to output buffer in correct order */
                    while (hex_pos > 0) {
                        temp_buffer[written++] = hex_buffer[--hex_pos];
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    temp_buffer[written++] = c;
                    break;
                }
                case '%': {
                    temp_buffer[written++] = '%';
                    break;
                }
                default:
                    /* Unsupported format specifier, just output it literally */
                    temp_buffer[written++] = '%';
                    temp_buffer[written++] = format[i];
                    break;
            }
        } else {
            temp_buffer[written++] = format[i];
        }
        i++;

        /* Check buffer limit */
        if (written >= DEBUG_BUFFER_SIZE - 1) {
            break;
        }
    }

    /* Null terminate the temporary buffer */
    temp_buffer[written] = '\0';

    /* Copy to destination with size checking */
    size_t copy_len = written < size - 1 ? written : size - 1;
    memcpy(str, temp_buffer, copy_len);
    str[copy_len] = '\0';

    return written;
}

/* Custom implementation of sprintf */
int custom_sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, DEBUG_BUFFER_SIZE, format, args);
    va_end(args);
    return result;
}

/* Format a debug message with level prefix and output it */
static void debug_format_and_write(int level, const char* format, va_list args) {
    /* Check if this level should be logged */
    if (level > debug_level) {
        return;
    }

    /* Prepare the prefix based on the level */
    const char* prefix = (level >= DEBUG_LEVEL_NONE && level <= DEBUG_LEVEL_TRACE) ?
                         level_prefix[level] : "";

    /* Copy prefix to buffer */
    size_t prefix_len = strlen(prefix);
    if (prefix_len >= DEBUG_BUFFER_SIZE) {
        prefix_len = DEBUG_BUFFER_SIZE - 1;
    }
    memcpy(debug_buffer, prefix, prefix_len);

    /* Format the message */
    int message_len = vsnprintf(debug_buffer + prefix_len,
                               DEBUG_BUFFER_SIZE - prefix_len - 2, /* -2 for \r\n */
                               format, args);

    if (message_len < 0) {
        /* vsnprintf error */
        return;
    }

    /* Calculate total length ensuring we don't exceed buffer */
    size_t total_len = prefix_len + message_len;
    if (total_len > DEBUG_BUFFER_SIZE - 3) { /* -3 for \r\n\0 */
        total_len = DEBUG_BUFFER_SIZE - 3;
    }

    /* Add newline at the end (CRLF for serial console) */
    debug_buffer[total_len] = '\r';
    debug_buffer[total_len + 1] = '\n';
    debug_buffer[total_len + 2] = '\0';

    /* Output the message - should now avoid duplicate serial output */
    debug_write(debug_buffer);
}

/* Debug output functions for different levels */
void debug_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_format_and_write(DEBUG_LEVEL_ERROR, format, args);
    va_end(args);
}

void debug_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_format_and_write(DEBUG_LEVEL_WARNING, format, args);
    va_end(args);
}

void debug_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_format_and_write(DEBUG_LEVEL_INFO, format, args);
    va_end(args);
}

void debug_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_format_and_write(DEBUG_LEVEL_DEBUG, format, args);
    va_end(args);
}

void debug_trace(const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_format_and_write(DEBUG_LEVEL_TRACE, format, args);
    va_end(args);
}

/* Generic debug output function with specified level */
void debug_log(int level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_format_and_write(level, format, args);
    va_end(args);
}

/* Raw debug output function (bypasses level checks) */
void debug_print(const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_vprint(format, args);
    va_end(args);
}

void debug_vprint(const char* format, va_list args) {
    /* Format the message */
    int message_len = vsnprintf(debug_buffer, DEBUG_BUFFER_SIZE - 1, format, args);

    if (message_len < 0) {
        /* vsnprintf error */
        return;
    }

    /* Ensure proper null termination */
    if (message_len >= DEBUG_BUFFER_SIZE) {
        message_len = DEBUG_BUFFER_SIZE - 1;
    }
    debug_buffer[message_len] = '\0';

    /* Output the message */
    debug_write(debug_buffer);
}

/* Custom implementation of sprintf and snprintf */
int snprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, size, format, args);
    va_end(args);
    return result;
}

void debug_hex_dump(const void* data, size_t size) {
    const uint8_t* bytes = (const uint8_t*)data;

    for (size_t i = 0; i < size; i += 16) {
        /* Build a complete line in one buffer */
        char line[120] = {0};
        int pos = 0;

        /* Add address prefix */
        pos += snprintf(line + pos, sizeof(line) - pos, "0x%08x: ", (uint32_t)i);

        /* Add hex part */
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                pos += snprintf(line + pos, sizeof(line) - pos, "%02x ", bytes[i + j]);
            } else {
                pos += snprintf(line + pos, sizeof(line) - pos, "   ");
            }

            /* Add extra space in the middle */
            if (j == 7) {
                pos += snprintf(line + pos, sizeof(line) - pos, " ");
            }
        }

        /* Add ASCII part */
        pos += snprintf(line + pos, sizeof(line) - pos, " |");
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                uint8_t c = bytes[i + j];
                if (c >= 32 && c <= 126) {
                    pos += snprintf(line + pos, sizeof(line) - pos, "%c", c);
                } else {
                    pos += snprintf(line + pos, sizeof(line) - pos, ".");
                }
            } else {
                pos += snprintf(line + pos, sizeof(line) - pos, " ");
            }
        }
        pos += snprintf(line + pos, sizeof(line) - pos, "|");

        /* Send the complete line to debug output */
        debug_print(line);
    }
}
