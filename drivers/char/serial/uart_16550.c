// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "serial: " fmt

#include <io.h>
#include <lib/string.h>
#include <seren/console.h>
#include <seren/init.h>
#include <seren/printk.h>
#include <seren/spinlock.h>

#define COM1_PORT 0x3F8

#define DATA_REG	0 // Data register (read/write)
#define INT_ENABLE_REG	1 // Interrupt Enable Register
#define FIFO_CTRL_REG	2 // FIFO Control Register
#define LINE_CTRL_REG	3 // Line Control Register
#define MODEM_CTRL_REG	4 // Modem Control Register
#define LINE_STATUS_REG 5 // Line Status Register

static spinlock_t serial_lock = SPIN_LOCK_UNLOCKED;

static int serial_hw_init(u16 port) {
	outb(port + INT_ENABLE_REG, 0x00);

	outb(port + LINE_CTRL_REG, 0x80);

	outb(port + DATA_REG, 0x03);
	outb(port + INT_ENABLE_REG, 0x00);

	outb(port + LINE_CTRL_REG, 0x03);

	outb(port + FIFO_CTRL_REG, 0xC7);

	outb(port + MODEM_CTRL_REG, 0x0B);

	return 0;
}

static int is_transmit_empty(u16 port) {
	return inb(port + LINE_STATUS_REG) & 0x20;
}

static void serial_putchar(u16 port, char a) {
	while (is_transmit_empty(port) == 0)
		outb(port, a);
}

static void serial_console_write(const char *buf, unsigned int len) {
	u64 flags;
	spin_lock_irqsave(&serial_lock, flags);
	for (unsigned int i = 0; i < len; i++) {
		serial_putchar(COM1_PORT, buf[i]);
	}
	spin_unlock_irqrestore(&serial_lock, flags);
}

static struct console serial_console = {
    .name = "ttyS0",
    .write = serial_console_write,
    .next = NULL,
};

static int __init setup_serial(void) {
	serial_hw_init(COM1_PORT);
	register_console(&serial_console);
	pr_info("uart_16550 initialized on COM1 and registered as console\n");
	return 0;
}

pure_initcall(setup_serial);