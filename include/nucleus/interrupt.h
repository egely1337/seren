#ifndef NUCLEUS_INTERRUPT_H
#define NUCLEUS_INTERRUPT_H

#include <lib/stdint.h>

typedef struct irq_context {

    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;

    uint64_t vector_number;
    uint64_t error_code;

    uint64_t rip_cpu;
    uint64_t cs_cpu;
    uint64_t rflags_cpu;
    uint64_t rsp_cpu;
    uint64_t ss_cpu;
} __attribute__((packed)) irq_context_t;

typedef void (*irq_c_handler_t)(irq_context_t *context);

/**
 * @brief Registers a C handler for a specific hardware IRQ line.
 * @param irq_line The original IRQ line number (0-15).
 * @param handler Pointer to the C function that will handle this IRQ.
 */
void interrupt_register_irq_handler(uint8_t irq_line, irq_c_handler_t handler);

/**
 * @brief Unregisters a C handler for a specific hardware IRQ line.
 * @param irq_line The original IRQ line number (0-15).
 */
void interrupt_unregister_irq_handler(uint8_t irq_line);

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
uint64_t interrupt_save_and_disable(void);

/**
 * @brief Restores a previously saved interrupts state.
 */
void interrupt_restore(uint64_t flags);

/**
 * @brief Unmasks a specific IRQ line.
 */
void irq_unmask(uint8_t irq_line);

#endif // NUCLEUS_INTERRUPT_H