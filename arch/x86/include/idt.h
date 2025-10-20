// SPDX-License-Identifier: Apache-2.0

#ifndef IDT_H
#define IDT_H

#include <seren/types.h>

/**
 * struct idt_entry - Interrupt Descriptor Table entry
 *
 * This structure defines a single 16-byte gate descriptor in the GDT.
 * It tells the CPU where to find the Interrupt Service Routine (ISR) for a
 * given interrupt vector, what code segment to use and what privilege level
 * is required.
 */
typedef struct idt_entry {
	u16 isr_low;
	u16 kernel_cs;
	u8 ist;
	u8 attributes;
	u16 isr_mid;
	u32 isr_high;
	u32 reserved;
} __attribute__((packed)) idt_entry_t;

/**
 * struct idt_ptr - Pointer structure for the LIDT instruction
 */
typedef struct idt_ptr {
	u16 limit;
	u64 base;
} __attribute__((packed)) idt_ptr_t;

/* x86 architecture supports up to 256 interrupt vectors */
#define IDT_MAX_DESCRIPTORS 256

#define IDT_ATTR_PRESENT 0x80
#define IDT_ATTR_RING0	 0x00
#define IDT_ATTR_RING3	 0x60

#define IDT_TYPE_INTERRUPT_GATE 0x0E
#define IDT_TYPE_TRAP_GATE	0x0F

/**
 * idt_init - Initializes the Interrupt Descriptor Table.
 */
void idt_init(void);

/**
 * idt_set_gate - Configure a singe IDT entry.
 * @vector_num: The interrupt vector number (0-255).
 * @isr_address: The 64-bit address of the interrupt handler.
 * @cs_selector: The code segment selector from the GDT.
 * @attributes: The type and attribute flags.
 * @ist: The Interrupt Stack Table index (if any).
 */
void idt_set_gate(u8 vector_num, u64 isr_address, u16 cs_selector,
		  u8 attributes, u8 ist);

#endif // IDT_H