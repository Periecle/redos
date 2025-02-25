#include <stdint.h>
#include <stdlib.h>

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn))
void __stack_chk_fail(void)
{
#if __STDC_HOSTED__
    abort();
#elif __is_redos_kernel
    /* Call the panic function, which doesn't return */
    extern void panic(const char* message) __attribute__((noreturn));
    panic("Stack smashing detected");
#else
    /* In case there's no panic function available, just enter an infinite loop */
    while (1) { }
    __builtin_unreachable();
#endif
}
