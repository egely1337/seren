// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_INTERRUPT_H
#define _SEREN_INTERRUPT_H

#include <asm/irqflags.h>
#include <asm/ptrace.h>
#include <seren/types.h>

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

#endif // _SEREN_INTERRUPT_H