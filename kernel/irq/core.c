// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "irq: " fmt

#include <pic.h>
#include <seren/interrupt.h>
#include <seren/printk.h>
#include <seren/sched/sched.h>
#include <seren/stddef.h>
#include <seren/types.h>

static irq_handler_t __irq_handlers[16] = {NULL};

void do_irq(struct pt_regs *regs) {
	u32 irq = regs->vector - PIC_IRQ_OFFSET_MASTER;

	if (irq == 7 && !(pic_read_isr() & (1 << 7))) {
		pr_debug("spurious IRQ7 detected, ignoring\n");
		return;
	}

	irq_handler_t handler = __irq_handlers[irq];

	if (likely(handler))
		__irq_handlers[irq](regs);
	else
		pr_warn("unhandled IRQ %u\n", irq);

	if (irq >= 8) {
		pic_send_eoi(8);
	}
	pic_send_eoi(irq);
}

int request_irq(u32 irq, irq_handler_t handler) {
	unsigned long flags;

	if (unlikely(irq >= 16 || !handler)) {
		return -1;
	}

	flags = local_irq_save();
	if (__irq_handlers[irq]) {
		local_irq_restore(flags);
		return -1;
	}

	__irq_handlers[irq] = handler;
	local_irq_restore(flags);

	enable_irq(irq);
	pr_debug("registered handler for IRQ %u\n", irq);

	return 0;
}

void free_irq(u32 irq) {
	unsigned long flags;

	if (unlikely(irq >= 16))
		return;

	disable_irq(irq);
	flags = local_irq_save();
	__irq_handlers[irq] = NULL;
	local_irq_restore(flags);

	pr_debug("unregistered handler for IRQ %u\n", irq);
}
