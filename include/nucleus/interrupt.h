// SPDX-License-Identifier: Apache-2.0

#ifndef _NUCLEUS_INTERRUPT_H
#define _NUCLEUS_INTERRUPT_H

#include <asm/irqflags.h>
#include <nucleus/types.h>

struct pt_regs {
	u64 r15, r14, r13, r12, r11, r10, r9, r8;
	u64 rdi, rsi, rbp, rbx, rdx, rcx, rax;

	u64 vector;
	u64 error_code;

	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
};

typedef void (*irq_handler_t)(struct pt_regs *regs);

/**
 * request_irq - Register a handler for a hardware interrupt
 * @irq:        The IRQ line number
 * @handler:    The function to be called
 */
int request_irq(u32 irq, irq_handler_t handler);

/*
 * free_irq - Unregister a handler for a hardware interrupt
 * @irq:    The IRQ line number (0-15)
 */
void free_irq(u32 irq);

/*
 * enable_irq - Unmask an IRQ line at the interrupt controller
 * @irq:    The IRQ line to enable
 */
void enable_irq(u32 irq);

/*
 * disable_irq - Mask an IRQ line at the interrupt controller
 * @irq:    The IRQ line to disable
 */
void disable_irq(u32 irq);

#endif // _NUCLEUS_INTERRUPT_H