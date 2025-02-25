#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <kernel/debug.h>

/* Function to dump registers for debugging (called by panic handlers) */
void dump_registers(void) {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp, eflags;
    uint32_t cr0, cr2, cr3, cr4;

    /* Get general purpose registers */
    __asm__ volatile (
        "movl %%eax, %0\n"
        "movl %%ebx, %1\n"
        "movl %%ecx, %2\n"
        "movl %%edx, %3\n"
        "movl %%esi, %4\n"
        "movl %%edi, %5\n"
        "movl %%ebp, %6\n"
        "movl %%esp, %7\n"
        "pushfl\n"
        "popl %8\n"
        : "=m"(eax), "=m"(ebx), "=m"(ecx), "=m"(edx),
          "=m"(esi), "=m"(edi), "=m"(ebp), "=m"(esp),
          "=m"(eflags)
        :
        : "memory"
    );

    /* Get control registers separately to avoid assembly errors */
    __asm__ volatile ("movl %%cr0, %%eax; movl %%eax, %0" : "=m"(cr0) : : "eax");
    __asm__ volatile ("movl %%cr2, %%eax; movl %%eax, %0" : "=m"(cr2) : : "eax");
    __asm__ volatile ("movl %%cr3, %%eax; movl %%eax, %0" : "=m"(cr3) : : "eax");
    __asm__ volatile ("movl %%cr4, %%eax; movl %%eax, %0" : "=m"(cr4) : : "eax");

    /* Get approximate EIP (this will be the address of the label below) */
    uint32_t eip;
    __asm__ volatile ("1: movl $1b, %0" : "=r"(eip));

    /* Print register values */
    debug_error("Register dump:");
    debug_error("EAX: %x    EBX: %x    ECX: %x    EDX: %x", eax, ebx, ecx, edx);
    debug_error("ESI: %x    EDI: %x    EBP: %x    ESP: %x", esi, edi, ebp, esp);
    debug_error("EIP: %x    EFLAGS: %x", eip, eflags);
    debug_error("CR0: %x    CR2: %x    CR3: %x    CR4: %x", cr0, cr2, cr3, cr4);
}

/* Kernel panic handler - prints message and halts */
__attribute__((noreturn))
void panic(const char* message) {
    /* Disable interrupts */
    __asm__ volatile("cli");

    /* Print panic message to all available outputs */
    debug_set_target(DEBUG_TARGET_ALL);

    /* Print attention-grabbing header */
    debug_error("************************************************************");
    debug_error("*                      KERNEL PANIC                        *");
    debug_error("************************************************************");

    /* Print the panic message */
    debug_error("PANIC: %s", message);

    /* Dump registers to aid debugging */
    dump_registers();

    /* Print system halt message */
    debug_error("System halted.");

    /* Endless loop to halt the system */
    while (1) {
        /* Halt the CPU */
        __asm__ volatile("hlt");
    }

    /* This will never be reached, but ensures the compiler knows this function doesn't return */
    __builtin_unreachable();
}

/* Kernel panic handler with formatted message */
__attribute__((noreturn))
void panicf(const char* format, ...) {
    char buffer[256];
    va_list args;

    /* Format the message */
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    /* Call the main panic handler with the formatted message */
    panic(buffer);
}

/* Handler for unhandled exceptions */
__attribute__((noreturn))
void exception_handler(uint32_t exception_number, uint32_t error_code) {
    /* Define exception names */
    static const char* exception_names[] = {
        "Divide Error",
        "Debug Exception",
        "NMI Interrupt",
        "Breakpoint",
        "Overflow",
        "BOUND Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection",
        "Page Fault",
        "Reserved",
        "x87 FPU Floating-Point Error",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Control Protection Exception",
        "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
        "Reserved", "Reserved", "Reserved", "Reserved", "Reserved"
    };

    /* Call panic with the exception information */
    if (exception_number < sizeof(exception_names) / sizeof(exception_names[0])) {
        panicf("Exception %d (%s), Error Code: %x",
              exception_number, exception_names[exception_number], error_code);
    } else {
        panicf("Unknown Exception %d, Error Code: %x", exception_number, error_code);
    }
}
