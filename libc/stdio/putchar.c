#include <stdio.h>

#if defined(__is_libc)
#include "../../kernel/include/kernel/tty.h"
#endif

int putchar(int ic) {
#if defined(__is_libc)
	char c = (char) ic;
	terminal_write(&c, sizeof(c));
#else
	// TODO: Implement stdio and the write system call.
#endif
	return ic;
}
