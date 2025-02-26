#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <kernel/debug.h>

/* Function to dump registers for debugging (called by panic handlers) */
void dump_registers(void) {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp, r8, r9, r10, r11, r12, r13, r14, r15, rflags;
    uint64_t cr0, cr2, cr3, cr4;

    /* Get general purpose registers */
    __asm__ volatile (
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        "movq %%r8, %8\n"
        "movq %%r9, %9\n"
        "movq %%r10, %10\n"
        "movq %%r11, %11\n"
        "movq %%r12, %12\n"
        "movq %%r13, %13\n"
        "movq %%r14, %14\n"
        "movq %%r15, %15\n"
        "pushfq\n"
        "popq %16\n"
        : "=m"(rax), "=m"(rbx), "=m"(rcx), "=m"(rdx),
          "=m"(rsi), "=m"(rdi), "=m"(rbp), "=m"(rsp),
          "=m"(r8), "=m"(r9), "=m"(r10), "=m"(r11),
          "=m"(r12), "=m"(r13), "=m"(r14), "=m"(r15),
          "=m"(rflags)
        :
        : "memory"
    );

    /* Get control registers separately to avoid assembly errors */
    __asm__ volatile ("movq %%cr0, %%rax; movq %%rax, %0" : "=m"(cr0) : : "rax");
    __asm__ volatile ("movq %%cr2, %%rax; movq %%rax, %0" : "=m"(cr2) : : "rax");
    __asm__ volatile ("movq %%cr3, %%rax; movq %%rax, %0" : "=m"(cr3) : : "rax");
    __asm__ volatile ("movq %%cr4, %%rax; movq %%rax, %0" : "=m"(cr4) : : "rax");

    /* Get approximate RIP (this will be the address of the label below) */
    uint64_t rip;
    __asm__ volatile ("leaq 1f(%%rip), %0; 1:" : "=r"(rip));

    /* Print register values */
    debug_error("Register dump:");
    debug_error("RAX: %lx    RBX: %lx    RCX: %lx    RDX: %lx", rax, rbx, rcx, rdx);
    debug_error("RSI: %lx    RDI: %lx    RBP: %lx    RSP: %lx", rsi, rdi, rbp, rsp);
    debug_error("R8:  %lx    R9:  %lx    R10: %lx    R11: %lx", r8, r9, r10, r11);
    debug_error("R12: %lx    R13: %lx    R14: %lx    R15: %lx", r12, r13, r14, r15);
    debug_error("RIP: %lx    RFLAGS: %lx", rip, rflags);
    debug_error("CR0: %lx    CR2: %lx    CR3: %lx    CR4: %lx", cr0, cr2, cr3, cr4);
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
void exception_handler(uint64_t exception_number, uint64_t error_code) {
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
        panicf("Exception %lu (%s), Error Code: %lx",
              exception_number, exception_names[exception_number], error_code);
    } else {
        panicf("Unknown Exception %lu, Error Code: %lx", exception_number, error_code);
    }
}
