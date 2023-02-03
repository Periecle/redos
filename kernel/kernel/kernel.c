#include <stdio.h>

#include <kernel/tty.h>

void kernel_main(void) {
	terminal_initialize();
	printf("Terminal Initialized Successfully!\n");
    printf("Character: %c %c \n", 'r', 80);
    printf("String: %s \n", "I am test string");
    printf("Decimal: %d %d %d \n", 540 + 4242, -231, 0);
    printf("Hexadecimal: %x %x %x \n", 550, 0, -550);
    printf("Octal: %o %o %o \n", 550, 0, -550);
    printf("Float: %f %f %f %f %f \n", -123.456, 123.456, (double) 0, 0.123, -0.456);

    int a = 51;

    printf("Pointer: %p \n", &a);
}
