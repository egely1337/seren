// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 egely1337, Arda Yetistiren
 */

#define pr_fmt(fmt) "pit: " fmt

#include <io.h>
#include <pic.h>
#include <seren/init.h>
#include <seren/interrupt.h>
#include <seren/pit.h>
#include <seren/printk.h>

static volatile u64 g_system_ticks = 0;

void timer_handler(void) {
	g_system_ticks++;
	pic_send_eoi(0);
}

void timer_init(void) {
	u32 frequency = 100; // 100 Hz
	u16 divisor = TIMER_FREQUENCY / frequency;

	outb(0x43, 0x36);

	outb(0x40, (u8)divisor & 0xFF);
	outb(0x40, (u8)(divisor >> 8) & 0xFF);

	request_irq(TIMER_IRQ, (irq_handler_t)timer_handler);
	pr_info("initialized with %u Hz frequency\n", frequency);
}

u64 timer_get_uptime_ms(void) { return g_system_ticks * 10; }

static int __init setup_timer(void) {
	timer_init();

	pic_unmask_irq(0);

	return 0;
}

device_initcall(setup_timer);
