/* SPDX-License-Identifier: Apache-2.0 */
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "traps: " fmt

#include <asm/irq_vectors.h>
#include <pic.h>
#include <seren/interrupt.h>
#include <seren/panic.h>
#include <seren/pit.h>
#include <seren/printk.h>
#include <seren/spinlock.h>
#include <seren/stddef.h>

static irq_handler_t irq_handlers[NR_IRQS] = {NULL};
static spinlock_t irq_handlers_lock = SPIN_LOCK_UNLOCKED;

static const char *exception_messages[] = {
    [DIVIDE_ERROR_VECTOR] = "Divide by Zero Error",
    [DEBUG_VECTOR] = "Debug",
    [NMI_VECTOR] = "Non-Maskable Interrupt",
    [BREAKPOINT_VECTOR] = "Breakpoint",
    [OVERFLOW_VECTOR] = "Overflow",
    [BOUNDS_CHECK_VECTOR] = "Bound Range Exceeded",
    [INVALID_OPCODE_VECTOR] = "Invalid Opcode",
    [DEVICE_NOT_AVAILABLE_VECTOR] = "Device Not Available",
    [DOUBLE_FAULT_VECTOR] = "Double Fault",
    [COPROCESSOR_SEGMENT_OVERRUN_VECTOR] = "Coprocessor Segment Overrun",
    [INVALID_TSS_VECTOR] = "Invalid TSS",
    [SEGMENT_NOT_PRESENT_VECTOR] = "Segment Not Present",
    [STACK_FAULT_VECTOR] = "Stack-Segment Fault",
    [GENERAL_PROTECTION_FAULT_VECTOR] = "General Protection Fault",
    [PAGE_FAULT_VECTOR] = "Page Fault",
    [X87_FPU_VECTOR] = "x87 Floating-Point Exception",
    [ALIGNMENT_CHECK_VECTOR] = "Alignment Check",
    [MACHINE_CHECK_VECTOR] = "Machine Check",
    [SIMD_FP_VECTOR] = "SIMD Floating-Point Exception",
    [VIRTUALIZATION_VECTOR] = "Virtualization Exception",
};

static void do_exception(struct pt_regs *regs) {
	const char *msg = "Unknown Exception";
	if (regs->vector < (sizeof(exception_messages) / sizeof(char *)) &&
	    exception_messages[regs->vector]) {
		msg = exception_messages[regs->vector];
	}
	die(msg, regs);
}

static int do_irq(struct pt_regs *regs) {
	int reschedule = 0;

	u32 irq = regs->vector - FIRST_EXTERNAL_VECTOR;

	if (irq == TIMER_IRQ) {
		reschedule = 1;
	}

	if (irq == 7 && !(pic_read_isr() & (1 << 7))) {
		pr_debug("spurious IRQ7 detected\n");
		return 0;
	}

	pic_send_eoi(irq);

	if (irq < NR_IRQS && irq_handlers[irq]) {
		irq_handlers[irq](regs);
	} else {
		pr_warn("unhandled IRQ %u on vector %llu\n", irq, regs->vector);
	}

	return reschedule;
}

int handle_interrupt(struct pt_regs *regs) {
	if (regs->vector < FIRST_EXTERNAL_VECTOR) {
		do_exception(regs);
		// Exceptions are fatal and never return
		return 0;
	} else {
		return do_irq(regs);
	}
}

int request_irq(u32 irq, irq_handler_t handler) {
	u64 flags;
	if (irq >= NR_IRQS || !handler)
		return -1;

	spin_lock_irqsave(&irq_handlers_lock, flags);
	if (irq_handlers[irq]) {
		spin_unlock_irqrestore(&irq_handlers_lock, flags);
		return -1; // Busy
	}
	irq_handlers[irq] = handler;
	spin_unlock_irqrestore(&irq_handlers_lock, flags);

	pic_unmask_irq(irq);
	return 0;
}

void free_irq(u32 irq) {
	u64 flags;
	if (unlikely(irq >= NR_IRQS))
		return;

	pic_mask_irq(irq);

	spin_lock_irqsave(&irq_handlers_lock, flags);
	irq_handlers[irq] = NULL;
	spin_unlock_irqrestore(&irq_handlers_lock, flags);
}