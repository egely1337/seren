// SPDX-License-Identifier: Apache-2.0

#ifndef ARCH_X86_64_ARCH_H
#define ARCH_X86_64_ARCH_H

/**
 * These are simple wrappers around common x86 assembly instructions.
 *
 * It's generally better to use the functions like local_irq_enable()/disabe()
 * from irqflags.h, but these can be useful for raw control.
 */

/* sti - Set Interrupt Flag (enable interrupts) */
#define sti() __asm__ volatile("sti");

/* cli - Clear Interrupt Flag (enable interrupts) */
#define cli() __asm__ volatile("cli");

/* hlt - Halt CPU until the next interrupt */
#define hlt() __asm__ volatile("hlt");

/**
 * arch_init - Main entry point for architecture-specific initialization.
 */
void arch_init(void);

#endif // ARCH_X86_64_ARCH_H