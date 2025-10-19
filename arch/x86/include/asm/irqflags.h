// SPDX-License-Identifier: Apache-2.0

#ifndef _ARCH_X86_64_ASM_IRQFLAGS_H
#define _ARCH_X86_64_ASM_IRQFLAGS_H

/**
 * local_irq_enable - Enable interrupts on the local CPU
 */
static inline void local_irq_enable(void) {
	__asm__ volatile("sti" ::: "memory");
}

/*
 * local_irq_disable - Disable interrupts on the local CPU
 */
static inline void local_irq_disable(void) {
	__asm__ volatile("cli" ::: "memory");
}

/*
 * local_irq_save - Save interrupt flags and disable interrupts
 *
 * Returns a the saved RFLAGS.
 */
static inline unsigned long local_irq_save(void) {
	unsigned long flags;
	__asm__ volatile("pushfq ; pop %0 ; cli" : "=r"(flags) : : "memory");
	return flags;
}

/*
 * local_irq_restore - Restore interrupt flags
 * @flags:  The RFLAGS returned from local_irq_save()
 */
static inline void local_irq_restore(unsigned long flags) {
	__asm__ volatile("push %0 ; popfq" : : "r"(flags) : "memory");
}

#endif // _ARCH_X86_64_ASM_IRQFLAGS_H