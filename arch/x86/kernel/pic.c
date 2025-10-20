// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 *
 * This is the driver for the venerbale Intel 8259 Programmable
 * Interrupt Controller (PIC).
 */

#include <io.h>
#include <pic.h>
#include <seren/printk.h>

/* Initialization Command Words (ICW) and Operational Command Words (OCWs) */
#define ICW1_INIT	  0x10
#define ICW1_ICW4_NEEDED  0x01
#define PIC_OCW3_READ_ISR 0x0B
#define ICW4_8086_MODE	  0x01

void pic_init(void) {
	/**
	 * Start the initialization sequence (ICW1) for both PICs.
	 * We tell them to expect an ICW4 command later on.
	 */
	outb(PIC1_COMMAND_PORT, ICW1_INIT | ICW1_ICW4_NEEDED);
	io_wait();
	outb(PIC2_COMMAND_PORT, ICW1_INIT | ICW1_ICW4_NEEDED);
	io_wait();

	/**
	 * ICW2: Set the vector offsets.
	 */
	outb(PIC1_DATA_PORT, PIC_IRQ_OFFSET_MASTER);
	io_wait();
	outb(PIC2_DATA_PORT, PIC_IRQ_OFFSET_SLAVE);
	io_wait();

	/**
	 * ICW3: Set up the master-slave relationship.
	 * Tell the Master that a slave is connected on its IRQ line 2 (0x04).
	 * Tell the Slave its cascade identity is 2 (0x0x2).
	 */
	outb(PIC1_DATA_PORT, 0x04);
	io_wait();
	outb(PIC2_DATA_PORT, 0x02);
	io_wait();

	/**
	 * ICW4: Set the PICs to 8086/88 mode. We don't need any of the other
	 * features.
	 */
	outb(PIC1_DATA_PORT, ICW4_8086_MODE);
	io_wait();
	outb(PIC2_DATA_PORT, ICW4_8086_MODE);
	io_wait();

	/**
	 * Initialization is done. Now we mask all interrupts on both PICs.
	 * Drivers will be responsible for unmasking the specific IRQs they
	 * want to handle.
	 */
	outb(PIC1_DATA_PORT, 0xFF);
	outb(PIC2_DATA_PORT, 0xFF);

	pr_info("PICs remapped: Master (0x%x-0x%x), Slave (0x%x-0x%x)\n",
		PIC_IRQ_OFFSET_MASTER, PIC_IRQ_OFFSET_MASTER + 7,
		PIC_IRQ_OFFSET_SLAVE, PIC_IRQ_OFFSET_SLAVE + 7);
}

void pic_send_eoi(u8 irq_number) {
	/**
	 * If the interrupt came from the slave PIC, we have to
	 * send an EOI to it first.
	 */
	if (irq_number >= 8 && irq_number <= 15) {
		outb(PIC2_COMMAND_PORT, PIC_EOI);
	}

	/**
	 * We always send an EOI to the master PIC, even for slave interrupts.
	 */
	outb(PIC1_COMMAND_PORT, PIC_EOI);
}

/**
 * To mask an IRQ, we read the current mask, set the corresponding bit
 * and write it back.
 */
void pic_mask_irq(u8 irq_line) {
	u16 port;
	u8 value;

	if (irq_line < 8) {
		port = PIC1_DATA_PORT;
	} else if (irq_line < 16) {
		port = PIC2_DATA_PORT;
		irq_line -= 8;
	} else {
		return;
	}
	value = inb(port) | (1 << irq_line);
	outb(port, value);
}

/**
 * To unmask an IRQ, we read the current mask, clear the corresponding bit,
 * and write it back.
 */
void pic_unmask_irq(u8 irq_line) {
	u16 port;
	u8 value;

	if (irq_line < 8) {
		port = PIC1_DATA_PORT;
	} else if (irq_line < 16) {
		port = PIC2_DATA_PORT;
		irq_line -= 8;
	} else {
		return;
	}
	value = inb(port) & ~(1 << irq_line);
	outb(port, value);
}

u8 pic_read_isr(void) {
	outb(PIC1_COMMAND_PORT, PIC_OCW3_READ_ISR);
	return inb(PIC1_COMMAND_PORT);
}
