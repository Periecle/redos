#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern void serial_com1_write_byte(unsigned char byte);

/* Global flag to indicate if serial output is enabled */
static bool serial_output_enabled = false;

/* Enable serial output for printf */
void printf_enable_serial(bool enable) {
    serial_output_enabled = enable;
}

extern bool terminal_is_serial_enabled(void);


static bool print(const char *data, size_t length) {
    const unsigned char *bytes = (const unsigned char *) data;
    for (size_t i = 0; i < length; i++) {
        if (putchar(bytes[i]) == EOF)
            return false;

        /* Output to serial if enabled */
        if (serial_output_enabled && !terminal_is_serial_enabled()) {
            /* Add CR before LF for serial terminals */
            if (bytes[i] == '\n') {
                serial_com1_write_byte('\r');
            }
            serial_com1_write_byte(bytes[i]);
        }
    }
    return true;
}

int printf(const char *restrict format, ...) {
    va_list parameters;
    va_start(parameters, format);
    int written = 0;
    while (*format != '\0') {
        size_t maxrem = INT_MAX - written;
        if (format[0] != '%' || format[1] == '%') {
            if (format[0] == '%')
                format++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount) {
                return -1;
            }
            if (!print(format, amount))
                return -1;
            format += amount;
            written += amount;
            continue;
        }
        const char *format_begun_at = format++;
        const char zero = '0';
        const char minus = '-';
        if (*format == 'c') {
            format++;
            char c = (char) va_arg(parameters, int );
            if (!maxrem) {
                return -1;
            }
            if (!print(&c, sizeof(c)))
                return -1;
            written++;
        } else if (*format == 's') {
            format++;
            const char *str = va_arg(parameters, const char*);
            size_t len = strlen(str);
            if (maxrem < len) {
                return -1;
            }
            if (!print(str, len))
                return -1;
            written += len;
        } else if (*format == 'd') {
            format++;
            int num = va_arg(parameters, int);
            int i = 0, sign = 1;
            if (num == 0) {
                written++;
                putchar(zero);
                if (serial_output_enabled) {
                    serial_com1_write_byte(zero);
                }
            }
            if (num < 0) {
                sign = -1;
                num = -num;
            }
            char arr[32];
            while (num > 0) {
                arr[i++] = (num % 10) + '0';
                num /= 10;
                written++;
            }
            if (sign == -1) {
                written++;
                putchar(minus);
                if (serial_output_enabled) {
                    serial_com1_write_byte(minus);
                }
            }
            while (i > 0) {
                written++;
                char c = arr[--i];
                putchar(c);
                if (serial_output_enabled) {
                    serial_com1_write_byte(c);
                }
            }
        } else if (*format == 'x') {
            format++;
            int num = va_arg(parameters, int);
            int i = 0, sign = 1;
            char arr[32];

            if (num == 0) {
                const char *hex_zero = "0x0";
                written += 3;
                if (!print(hex_zero, 3))
                    return -1;
            } else {
                if (num < 0) {
                    sign = -1;
                    num = -num;
                }
                while (num > 0) {
                    int rem = num % 16;
                    if (rem < 10) {
                        arr[i++] = rem + '0';
                    } else {
                        arr[i++] = rem - 10 + 'A';
                    }
                    num /= 16;
                }
                if (sign == -1) {
                    written++;
                    putchar('-');
                    if (serial_output_enabled) {
                        serial_com1_write_byte('-');
                    }
                }
                putchar('0');
                putchar('x');
                written += 2;
                if (serial_output_enabled) {
                    serial_com1_write_byte('0');
                    serial_com1_write_byte('x');
                }
                while (i > 0) {
                    written++;
                    char c = arr[--i];
                    putchar(c);
                    if (serial_output_enabled) {
                        serial_com1_write_byte(c);
                    }
                }
            }
        } else if (*format == 'o') {
            format++;
            int num = va_arg(parameters, int);
            int i = 0, sign = 1;
            char arr[32];
            if (num == 0) {
                written++;
                putchar(zero);
                if (serial_output_enabled) {
                    serial_com1_write_byte(zero);
                }
            } else {
                if (num < 0) {
                    sign = -1;
                    num = -num;
                }
                while (num > 0) {
                    int rem = num % 8;
                    arr[i++] = rem + '0';
                    num /= 8;
                }
                if (sign == -1) {
                    written++;
                    putchar('-');
                    if (serial_output_enabled) {
                        serial_com1_write_byte('-');
                    }
                }
                putchar('0');
                written++;
                if (serial_output_enabled) {
                    serial_com1_write_byte('0');
                }
                while (i > 0) {
                    written++;
                    char c = arr[--i];
                    putchar(c);
                    if (serial_output_enabled) {
                        serial_com1_write_byte(c);
                    }
                }
            }
        } else if (*format == 'f') {
            format++;
            double num = va_arg(parameters, double);
            int sign = (num < 0) ? -1 : 1;
            num *= sign;
            int int_part = (int) num;
            double dec_part = num - int_part;
            int i = 0;
            char arr[32];
            if (sign == -1) {
                written++;
                putchar('-');
                if (serial_output_enabled) {
                    serial_com1_write_byte('-');
                }
            }
            if (int_part == 0) {
                written++;
                putchar('0');
                if (serial_output_enabled) {
                    serial_com1_write_byte('0');
                }
            }
            while (int_part > 0) {
                int rem = int_part % 10;
                arr[i++] = rem + '0';
                int_part /= 10;
            }
            while (i > 0) {
                written++;
                char c = arr[--i];
                putchar(c);
                if (serial_output_enabled) {
                    serial_com1_write_byte(c);
                }
            }
            if (dec_part > 0) {
                putchar('.');
                written++;
                if (serial_output_enabled) {
                    serial_com1_write_byte('.');
                }
                i = 0;
                while (dec_part > 0 && i < 5) {
                    dec_part *= 10;
                    int d = (int) dec_part;
                    arr[i++] = d + '0';
                    dec_part -= d;
                }
                i = 0;
                while (i < 5) {
                    written++;
                    char c = arr[i];
                    putchar(c);
                    if (serial_output_enabled) {
                        serial_com1_write_byte(c);
                    }
                    arr[i++] = 0;
                }
            }
        } else if (*format == 'p') {
            format++;
            void *pointer = va_arg(parameters, void*);
            const char *nil = "nil";
            if (pointer == NULL) {
                written += 3;
                if (!print(nil, 3))
                    return -1;
            } else {
                unsigned long int num = (unsigned long int) pointer;
                int i = 0;
                char arr[32];
                while (num > 0) {
                    int rem = num % 16;
                    if (rem < 10) {
                        arr[i++] = rem + '0';
                    } else {
                        arr[i++] = rem - 10 + 'A';
                    }
                    num /= 16;
                }
                putchar('0');
                putchar('x');
                written += 2;
                if (serial_output_enabled) {
                    serial_com1_write_byte('0');
                    serial_com1_write_byte('x');
                }
                while (i > 0) {
                    written++;
                    char c = arr[--i];
                    putchar(c);
                    if (serial_output_enabled) {
                        serial_com1_write_byte(c);
                    }
                }
            }
        } else {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len) {
                return -1;
            }
            if (!print(format, len))
                return -1;
            written += len;
            format += len;
        }
    }
    va_end(parameters);
    return written;
}
