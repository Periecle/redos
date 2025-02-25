#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern void serial_com1_write_byte(unsigned char byte);
static bool serial_output_enabled = false;

void printf_enable_serial(bool enable) {
    serial_output_enabled = enable;
}

extern bool terminal_is_serial_enabled(void);

static bool print(const char *data, size_t length) {
    const unsigned char *bytes = (const unsigned char *) data;
    for (size_t i = 0; i < length; i++) {
        if (putchar(bytes[i]) == EOF)
            return false;
        if (serial_output_enabled && !terminal_is_serial_enabled()) {
            if (bytes[i] == '\n') {
                serial_com1_write_byte('\r');
            }
            serial_com1_write_byte(bytes[i]);
        }
    }
    return true;
}

/* Helper function to convert integer to string */
static int itoa(char *str, int num, int base) {
    int i = 0;
    bool is_negative = false;

    /* Handle 0 explicitly */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    /* Handle negative numbers only for base 10 */
    if (num < 0 && base == 10) {
        is_negative = true;
        num = -num;
    }

    /* Process individual digits */
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'A');
        num = num / base;
    }

    /* If negative, append '-' */
    if (is_negative)
        str[i++] = '-';

    /* Reverse the string */
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    str[i] = '\0';
    return i;
}

/* Helper function to convert unsigned integer to string */
static int utoa(char *str, unsigned int num, int base) {
    int i = 0;

    /* Handle 0 explicitly */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    /* Process individual digits */
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'A');
        num = num / base;
    }

    /* Reverse the string */
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    str[i] = '\0';
    return i;
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

        if (*format == 'c') {
            format++;
            char c = (char) va_arg(parameters, int);
            if (!maxrem) {
                return -1;
            }
            if (!print(&c, sizeof(c)))
                return -1;
            written++;
        } else if (*format == 's') {
            format++;
            const char *str = va_arg(parameters, const char*);
            if (str == NULL) {
                str = "(null)";
            }
            size_t len = strlen(str);
            if (maxrem < len) {
                return -1;
            }
            if (!print(str, len))
                return -1;
            written += len;
        } else if (*format == 'd' || *format == 'i') {
            format++;
            int num = va_arg(parameters, int);

            char num_str[32];
            int len = itoa(num_str, num, 10);

            if (maxrem < (size_t)len) {
                return -1;
            }
            if (!print(num_str, len))
                return -1;
            written += len;
        } else if (*format == 'u') {
            format++;
            unsigned int num = va_arg(parameters, unsigned int);

            char num_str[32];
            int len = utoa(num_str, num, 10);

            if (maxrem < (size_t)len) {
                return -1;
            }
            if (!print(num_str, len))
                return -1;
            written += len;
        } else if (*format == 'x' || *format == 'X') {
            format++;
            unsigned int num = va_arg(parameters, unsigned int);

            char prefix[2] = {'0', 'x'};
            if (maxrem < 2) {
                return -1;
            }
            if (!print(prefix, 2))
                return -1;
            written += 2;

            char num_str[32];
            int len = utoa(num_str, num, 16);

            if (maxrem < (size_t)len + 2) {
                return -1;
            }
            if (!print(num_str, len))
                return -1;
            written += len;
        } else if (*format == 'p') {
            format++;
            void *ptr = va_arg(parameters, void*);

            if (ptr == NULL) {
                const char *nil = "(nil)";
                size_t len = strlen(nil);
                if (maxrem < len) {
                    return -1;
                }
                if (!print(nil, len))
                    return -1;
                written += len;
            } else {
                char prefix[2] = {'0', 'x'};
                if (maxrem < 2) {
                    return -1;
                }
                if (!print(prefix, 2))
                    return -1;
                written += 2;

                unsigned int ptr_val = (unsigned int)ptr;
                char num_str[32];
                int len = utoa(num_str, ptr_val, 16);

                if (maxrem < (size_t)len + 2) {
                    return -1;
                }
                if (!print(num_str, len))
                    return -1;
                written += len;
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
