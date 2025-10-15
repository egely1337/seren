#include <nucleus/interrupt.h>
#include <nucleus/types.h>
#include <pic.h>

void interrupts_enable(void) { __asm__ volatile("sti"); }
void interrupts_disable(void) { __asm__ volatile("cli"); }

uint64_t interrupt_save_and_disable(void) {
    uint64_t flags;
    __asm__ volatile("pushfq \n\t"
                     "pop %0 \n\t"
                     "cli"
                     : "=r"(flags)
                     :
                     : "memory");
    return flags;
}

void interrupt_restore(uint64_t flags) {
    __asm__ volatile("push %0 \n\t"
                     "popfq"
                     :
                     : "r"(flags)
                     : "memory");
}

void irq_unmask(uint8_t irq_line) { pic_unmask_irq(irq_line); }
