#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static bool print(const char *data, size_t length) {
    const unsigned char *bytes = (const unsigned char *) data;
    for (size_t i = 0; i < length; i++)
        if (putchar(bytes[i]) == EOF)
            return false;
    return true;
}

//TODO: Rewrite that monster and add support for all C-standard fomatting options and implement it in vprintf
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
                // TODO: Set errno to EOVERFLOW.
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
            char c = (char) va_arg(parameters, int /* char promotes to int */);
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW.
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
                // TODO: Set errno to EOVERFLOW.
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
            }
            while (i > 0) {
                written++;
                putchar(arr[--i]);
            }
        } else if (*format == 'x') {
            format++;
            int num = va_arg(parameters, int);
            int i = 0, sign = 1;
            const char *hexademical_zero = "0x0";
            char arr[32];
            if (num == 0) {
                written += 3;
                if (!print(hexademical_zero, sizeof(hexademical_zero)))
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
                }
                putchar('0');
                putchar('x');

                written += 2;
                while (i > 0) {
                    written++;
                    putchar(arr[--i]);
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
                }
                putchar('0');

                written++;
                while (i > 0) {
                    written++;
                    putchar(arr[--i]);
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
            }
            if (int_part == 0) {
                written++;
                putchar('0');
            }
            while (int_part > 0) {
                int rem = int_part % 10;
                arr[i++] = rem + '0';
                int_part /= 10;
            }
            while (i > 0) {
                written++;
                putchar(arr[--i]);
            }
            if (dec_part > 0) {
                putchar('.');
                written++;
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
                    putchar(arr[i]);
                    arr[i++] = 0;
                }
            }
        } else if (*format == 'p') {
            format++;
            void *pointer = va_arg(parameters, void*);

            const char *nil = "nil";
            if (pointer == NULL) {
                written += 3;
                if (!print(nil, sizeof(nil)))
                    return -1;
            } else {
                unsigned long int num = (unsigned long int) pointer;
                int i = 0;
                const char *hexademical_zero = "0x0";
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
                while (i > 0) {
                    written++;
                    putchar(arr[--i]);
                }
            }
        } else {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
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
