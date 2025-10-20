// SPDX-License-Identifier: Apache-2.0

#ifndef DRIVERS_PIC_H
#define DRIVERS_PIC_H

#include <seren/types.h>

/* I/O ports for the master and slave 8259 PICs. */
#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT	  0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT	  0xA1

/* End-of-Interrupt command code */
#define PIC_EOI 0x20

/**
 * We remap the PIC interrupts to avoid conflicting with CPU exceptions.
 * By default they overlap with vectors 8-15. We'll move them to a safe
 * range starting at vector 32 (0x20).
 */
#define PIC_IRQ_OFFSET_MASTER 0x20
#define PIC_IRQ_OFFSET_SLAVE  0x28

/**
 * pic_init - Initialize and remap the 8259 PICs.
 */
void pic_init(void);

/**
 * pic_send_eoi - Send an End-Of-Interrupt signal to the PIC(s).
 * @irq_number: The hardware IRQ number (0-15) that was handled.
 *
 * This MUST be called at the end of an interrupt handler to let the PIC
 * know it's okay to send more interrupts.
 */
void pic_send_eoi(u8 irq_number);

/**
 * pic_mask_irq - Mask (disable) a specific IRQ line.
 * @irq_line: The IRQ line (0-15) to disable.
 */
void pic_mask_irq(u8 irq_line);

/**
 * pic_unmask_irq - Unmask (enable) a specific IRQ line.
 * @irq_line: The IRQ line (0-15) to enable.
 */
void pic_unmask_irq(u8 irq_line);

/**
 * pic_read_isr - Read the PIC's In-Service Register.
 *
 * This tells us which IRQs are currently being serviced. We use
 * this for detecting spurious interrupts.
 */
u8 pic_read_isr(void);

#endif // DRIVERS_PIC_H