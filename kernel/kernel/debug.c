#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/debug.h>
#include "../arch/i386/serial.h"

static int debug_level = DEBUG_LEVEL_INFO;
static int debug_target = DEBUG_TARGET_VGA;
static bool debug_initialized = false;

#define DEBUG_BUFFER_SIZE 1024
static char debug_buffer[DEBUG_BUFFER_SIZE];

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

void debug_set_level(int level) {
    if (level >= DEBUG_LEVEL_NONE && level <= DEBUG_LEVEL_TRACE) {
        debug_level = level;
    }
}

int debug_get_level(void) {
    return debug_level;
}

void debug_set_target(int target) {
    debug_target = target;
}

int debug_get_target(void) {
    return debug_target;
}

static void debug_write(const char* str) {
    if (debug_target & DEBUG_TARGET_VGA) {
        terminal_writestring(str);
    }
    if ((debug_target & DEBUG_TARGET_SERIAL) && !terminal_is_serial_enabled()) {
        serial_com1_write_string(str);
    }
}

/* Improved vsnprintf implementation */
int vsnprintf(char* str, size_t size, const char* format, va_list args) {
    if (size == 0) {
        return 0;
    }

    if (size == 1) {
        str[0] = '\0';
        return 0;
    }

    size_t count = 0;
    size_t output_index = 0;

    while (*format && output_index < size - 1) {
        if (*format != '%') {
            str[output_index++] = *format++;
            count++;
            continue;
        }

        // Handle format specifier
        format++; // Skip '%'

        if (*format == '\0') {
            break;
        }

        if (*format == '%') {
            // Literal '%'
            str[output_index++] = '%';
            count++;
            format++;
            continue;
        }

        // Handle format specifiers
        switch (*format) {
            case 'd':
            case 'i': {
                // Signed integer
                int value = va_arg(args, int);
                char buffer[32];
                bool negative = false;

                if (value < 0) {
                    negative = true;
                    value = -value;
                }

                // Convert to string (reversed)
                size_t pos = 0;
                do {
                    buffer[pos++] = '0' + (value % 10);
                    value /= 10;
                } while (value && pos < sizeof(buffer) - 1);

                // Add minus sign if negative
                if (negative && pos < sizeof(buffer) - 1) {
                    buffer[pos++] = '-';
                }

                // Copy to output (in correct order)
                while (pos > 0 && output_index < size - 1) {
                    str[output_index++] = buffer[--pos];
                    count++;
                }
                break;
            }

            case 'u': {
                // Unsigned integer
                unsigned int value = va_arg(args, unsigned int);
                char buffer[32];

                // Convert to string (reversed)
                size_t pos = 0;
                do {
                    buffer[pos++] = '0' + (value % 10);
                    value /= 10;
                } while (value && pos < sizeof(buffer) - 1);

                // Copy to output (in correct order)
                while (pos > 0 && output_index < size - 1) {
                    str[output_index++] = buffer[--pos];
                    count++;
                }
                break;
            }

            case 'x': {
                // Hexadecimal (lowercase)
                unsigned int value = va_arg(args, unsigned int);

                // Output "0x" prefix
                if (output_index + 1 < size - 1) {
                    str[output_index++] = '0';
                    str[output_index++] = 'x';
                    count += 2;
                } else {
                    break;
                }

                // Handle zero value specially
                if (value == 0 && output_index < size - 1) {
                    str[output_index++] = '0';
                    count++;
                    break;
                }

                char buffer[32];

                // Convert to hex string (reversed)
                size_t pos = 0;
                do {
                    unsigned int digit = value & 0xF;
                    buffer[pos++] = digit < 10 ? '0' + digit : 'a' + (digit - 10);
                    value >>= 4;
                } while (value && pos < sizeof(buffer) - 1);

                // Copy to output (in correct order)
                while (pos > 0 && output_index < size - 1) {
                    str[output_index++] = buffer[--pos];
                    count++;
                }
                break;
            }

            case 'X': {
                // Hexadecimal (uppercase)
                unsigned int value = va_arg(args, unsigned int);

                // Output "0x" prefix
                if (output_index + 1 < size - 1) {
                    str[output_index++] = '0';
                    str[output_index++] = 'x';
                    count += 2;
                } else {
                    break;
                }

                // Handle zero value specially
                if (value == 0 && output_index < size - 1) {
                    str[output_index++] = '0';
                    count++;
                    break;
                }

                char buffer[32];

                // Convert to hex string (reversed)
                size_t pos = 0;
                do {
                    unsigned int digit = value & 0xF;
                    buffer[pos++] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
                    value >>= 4;
                } while (value && pos < sizeof(buffer) - 1);

                // Copy to output (in correct order)
                while (pos > 0 && output_index < size - 1) {
                    str[output_index++] = buffer[--pos];
                    count++;
                }
                break;
            }

            case 'p': {
                // Pointer
                void* ptr = va_arg(args, void*);

                if (ptr == NULL) {
                    // Handle NULL pointer
                    const char* null_str = "(nil)";
                    size_t len = strlen(null_str);
                    size_t to_copy = (output_index + len < size - 1) ? len : (size - 1 - output_index);

                    memcpy(&str[output_index], null_str, to_copy);
                    output_index += to_copy;
                    count += to_copy;
                } else {
                    // Output "0x" prefix
                    if (output_index + 1 < size - 1) {
                        str[output_index++] = '0';
                        str[output_index++] = 'x';
                        count += 2;
                    } else {
                        break;
                    }

                    unsigned int value = (unsigned int)ptr;
                    char buffer[32];

                    // Convert to hex string (reversed)
                    size_t pos = 0;
                    do {
                        unsigned int digit = value & 0xF;
                        buffer[pos++] = digit < 10 ? '0' + digit : 'a' + (digit - 10);
                        value >>= 4;
                    } while (value && pos < sizeof(buffer) - 1);

                    // Ensure minimum width of 8 digits for 32-bit pointers
                    while (pos < 8 && pos < sizeof(buffer) - 1) {
                        buffer[pos++] = '0';
                    }

                    // Copy to output (in correct order)
                    while (pos > 0 && output_index < size - 1) {
                        str[output_index++] = buffer[--pos];
                        count++;
                    }
                }
                break;
            }

            case 's': {
                // String
                const char* value = va_arg(args, const char*);

                if (value == NULL) {
                    value = "(null)";
                }

                while (*value && output_index < size - 1) {
                    str[output_index++] = *value++;
                    count++;
                }
                break;
            }

            case 'c': {
                // Character
                char value = (char)va_arg(args, int);

                if (output_index < size - 1) {
                    str[output_index++] = value;
                    count++;
                }
                break;
            }

            default:
                // Unsupported format specifier - output as-is
                if (output_index < size - 1) {
                    str[output_index++] = '%';
                    count++;
                }

                if (output_index < size - 1) {
                    str[output_index++] = *format;
                    count++;
                }
                break;
        }

        format++;
    }

    // Null-terminate the output
    str[output_index] = '\0';

    return count;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, size, format, args);
    va_end(args);
    return result;
}

static void debug_format_and_write(int level, const char* format, va_list args) {
    if (level > debug_level) {
        return;
    }

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

/* Improved hex dump implementation */
void debug_hex_dump(const void* data, size_t size) {
    const uint8_t* bytes = (const uint8_t*)data;

    for (size_t i = 0; i < size; i += 16) {
        // Build the address prefix
        char line[128];
        int line_pos = 0;

        // Format address
        line_pos += snprintf(line + line_pos, sizeof(line) - line_pos, "%08x: ", (unsigned int)(bytes + i));

        // Format hex values
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                line_pos += snprintf(line + line_pos, sizeof(line) - line_pos, "%02x ", bytes[i + j]);
            } else {
                line_pos += snprintf(line + line_pos, sizeof(line) - line_pos, "   ");
            }

            // Add extra space in the middle
            if (j == 7) {
                line_pos += snprintf(line + line_pos, sizeof(line) - line_pos, " ");
            }
        }

        // Format ASCII values
        line_pos += snprintf(line + line_pos, sizeof(line) - line_pos, "| ");

        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                uint8_t c = bytes[i + j];
                if (c >= 32 && c <= 126) {
                    line_pos += snprintf(line + line_pos, sizeof(line) - line_pos, "%c", c);
                } else {
                    line_pos += snprintf(line + line_pos, sizeof(line) - line_pos, ".");
                }
            }
        }

        line_pos += snprintf(line + line_pos, sizeof(line) - line_pos, " |");

        // Output the full line
        debug_print("%s\r\n", line);
    }
}
