#include <nucleus/interrupt.h>
#include <nucleus/panic.h>
#include <nucleus/printk.h>
#include <nucleus/types.h>

static void print_registers(irq_context_t *context) {
    if (!context) {
        pr_crit("  Register context not available.\n");
        return;
    }

    pr_crit("  RIP: 0x%016lx  RFLAGS: 0x%016lx\n", context->rip_cpu,
            context->rflags_cpu);
    pr_crit("  RAX: 0x%016lx  RBX: 0x%016lx  RCX: 0x%016lx\n", context->rax,
            context->rbx, context->rcx);
    pr_crit("  RDX: 0x%016lx  RSI: 0x%016lx  RDI: 0x%016lx\n", context->rdx,
            context->rsi, context->rdi);
    pr_crit("  RBP: 0x%016lx  RSP: 0x%016lx\n", context->rbp, context->rsp_cpu);
    pr_crit("  R8:  0x%016lx  R9:  0x%016lx  R10: 0x%016lx\n", context->r8,
            context->r9, context->r10);
    pr_crit("  R11: 0x%016lx  R12: 0x%016lx  R13: 0x%016lx\n", context->r11,
            context->r12, context->r13);
    pr_crit("  R14: 0x%016lx  R15: 0x%016lx\n", context->r14, context->r15);
    pr_crit("  CS:  0x%04lx          SS:  0x%04lx\n", context->cs_cpu,
            context->ss_cpu);

    if (context->error_code != 0) {
        pr_crit("  Error Code: 0x%lx\n", context->error_code);
    }
}

void panic(const char *message, irq_context_t *context) {
    interrupts_disable();

    pr_emerg("==============================================================="
             "=======\n");
    pr_emerg("!!                        KERNEL PANIC                           "
             "   !!\n");
    pr_emerg("================================================================="
             "=====\n");

    if (message) {
        pr_emerg("Reason: %s\n\n", message);
    } else {
        pr_emerg("Reason: Unknown");
    }

    pr_crit("Register Dump:\n");
    print_registers(context);

    pr_emerg("\nSystem halted. Please reboot.\n");
    while (1) {
        __asm__ volatile("hlt");
    }
}