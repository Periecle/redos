#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <stdbool.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

/* Serial output control for printf */
void printf_enable_serial(bool enable);

#ifdef __cplusplus
}
#endif

#endif
