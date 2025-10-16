#ifndef NUCLEUS_INTERRUPT_H
#define NUCLEUS_INTERRUPT_H

#include <nucleus/types.h>

typedef struct irq_context {

    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rdi, rsi, rbp, rbx, rdx, rcx, rax;

    u64 vector_number;
    u64 error_code;

    u64 rip_cpu;
    u64 cs_cpu;
    u64 rflags_cpu;
    u64 rsp_cpu;
    u64 ss_cpu;
} __attribute__((packed)) irq_context_t;

typedef void (*irq_c_handler_t)(irq_context_t *context);

/**
 * @brief Registers a C handler for a specific hardware IRQ line.
 * @param irq_line The original IRQ line number (0-15).
 * @param handler Pointer to the C function that will handle this IRQ.
 */
void interrupt_register_irq_handler(u8 irq_line, irq_c_handler_t handler);

/**
 * @brief Unregisters a C handler for a specific hardware IRQ line.
 * @param irq_line The original IRQ line number (0-15).
 */
void interrupt_unregister_irq_handler(u8 irq_line);

/**
 * @brief The main C dispatcher for hardware IRQs.
 * This function is called by the assembly stub common_irq_dispatcher_stub.
 * It should NOT be called directly from other C code.
 * @param frame Pointer to the irq_context_t structure on the stack.
 */
void irq_c_dispatcher(irq_context_t *frame);

/**
 * @brief Enables interrupts.
 */
void interrupts_enable(void);

/**
 * @brief Disables interrupts.
 */
void interrupts_disable(void);

/**
 * @brief Disables interrupts and returns previous interrupt state.
 */
u64 interrupt_save_and_disable(void);

/**
 * @brief Restores a previously saved interrupts state.
 */
void interrupt_restore(u64 flags);

/**
 * @brief Unmasks a specific IRQ line.
 */
void irq_unmask(u8 irq_line);

#endif // NUCLEUS_INTERRUPT_H