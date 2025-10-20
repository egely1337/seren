// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "idt: " fmt

#include <asm/gdt.h>
#include <asm/irq_vectors.h>
#include <idt.h>
#include <pic.h>
#include <seren/printk.h>
#include <seren/types.h>

#define DECLARE_ISR(n) extern void isr##n(void)
#define DECLARE_IRQ(n) extern void irq_stub_##n(void)

// clang-format off

// Exceptions
DECLARE_ISR(0); DECLARE_ISR(1); DECLARE_ISR(2); DECLARE_ISR(3); DECLARE_ISR(4);
DECLARE_ISR(5); DECLARE_ISR(6); DECLARE_ISR(7); DECLARE_ISR(8); DECLARE_ISR(9);
DECLARE_ISR(10); DECLARE_ISR(11); DECLARE_ISR(12); DECLARE_ISR(13); DECLARE_ISR(14);
DECLARE_ISR(16); DECLARE_ISR(17); DECLARE_ISR(18); DECLARE_ISR(19); DECLARE_ISR(20);

// IRQs
DECLARE_IRQ(0); DECLARE_IRQ(1); DECLARE_IRQ(2); DECLARE_IRQ(3); DECLARE_IRQ(4);
DECLARE_IRQ(5); DECLARE_IRQ(6); DECLARE_IRQ(7); DECLARE_IRQ(8); DECLARE_IRQ(9);
DECLARE_IRQ(10); DECLARE_IRQ(11); DECLARE_IRQ(12); DECLARE_IRQ(13); DECLARE_IRQ(14);
DECLARE_IRQ(15);

// clang-format on

struct idt_init_entry {
	u8 vector;
	void (*handler)(void);
	u8 ist;
};

static const struct idt_init_entry idt_entries[] = {
    {DIVIDE_ERROR_VECTOR, isr0, 0},
    {DEBUG_VECTOR, isr1, 0},
    {NMI_VECTOR, isr2, 0},
    {BREAKPOINT_VECTOR, isr3, 0},
    {OVERFLOW_VECTOR, isr4, 0},
    {BOUNDS_CHECK_VECTOR, isr5, 0},
    {INVALID_OPCODE_VECTOR, isr6, 0},
    {DEVICE_NOT_AVAILABLE_VECTOR, isr7, 0},
    {DOUBLE_FAULT_VECTOR, isr8, IST_DOUBLE_FAULT},
    {INVALID_TSS_VECTOR, isr10, 0},
    {SEGMENT_NOT_PRESENT_VECTOR, isr11, 0},
    {STACK_FAULT_VECTOR, isr12, 0},
    {GENERAL_PROTECTION_FAULT_VECTOR, isr13, 0},
    {PAGE_FAULT_VECTOR, isr14, 0},
    {X87_FPU_VECTOR, isr16, 0},
    {ALIGNMENT_CHECK_VECTOR, isr17, 0},
    {MACHINE_CHECK_VECTOR, isr18, 0},
    {SIMD_FP_VECTOR, isr19, 0},
    {VIRTUALIZATION_VECTOR, isr20, 0},

    {PIC1_START_VECTOR + 0, irq_stub_0, 0},
    {PIC1_START_VECTOR + 1, irq_stub_1, 0},
    {PIC1_START_VECTOR + 2, irq_stub_2, 0},
    {PIC1_START_VECTOR + 3, irq_stub_3, 0},
    {PIC1_START_VECTOR + 4, irq_stub_4, 0},
    {PIC1_START_VECTOR + 5, irq_stub_5, 0},
    {PIC1_START_VECTOR + 6, irq_stub_6, 0},
    {PIC1_START_VECTOR + 7, irq_stub_7, 0},
    {PIC2_START_VECTOR + 0, irq_stub_8, 0},
    {PIC2_START_VECTOR + 1, irq_stub_9, 0},
    {PIC2_START_VECTOR + 2, irq_stub_10, 0},
    {PIC2_START_VECTOR + 3, irq_stub_11, 0},
    {PIC2_START_VECTOR + 4, irq_stub_12, 0},
    {PIC2_START_VECTOR + 5, irq_stub_13, 0},
    {PIC2_START_VECTOR + 6, irq_stub_14, 0},
    {PIC2_START_VECTOR + 7, irq_stub_15, 0},
};

static idt_entry_t idt[IDT_MAX_DESCRIPTORS];
static idt_ptr_t idtp;

void idt_set_gate(u8 vector_num, u64 isr_address, u16 cs_selector,
		  u8 attributes, u8 ist) {
	idt_entry_t *descriptor = &idt[vector_num];

	descriptor->isr_low = (u16)(isr_address & 0xFFFF);
	descriptor->kernel_cs = cs_selector;
	descriptor->ist = ist;
	descriptor->attributes = attributes;
	descriptor->isr_mid = (u16)((isr_address >> 16) & 0xFFFF);
	descriptor->isr_high = (u32)((isr_address >> 32) & 0xFFFFFFFF);
	descriptor->reserved = 0;
}

extern void idt_load(idt_ptr_t *idtp_param);

void idt_init(void) {
	const u8 gate_attrs =
	    IDT_ATTR_PRESENT | IDT_TYPE_INTERRUPT_GATE | IDT_ATTR_RING0;

	pr_info("Initializing IDT...\n");

	for (size_t i = 0;
	     i < (sizeof(idt_entries) / sizeof(struct idt_init_entry)); i++) {
		const struct idt_init_entry *entry = &idt_entries[i];
		idt_set_gate(entry->vector, (u64)entry->handler,
			     GDT_KERNEL_CODE_SELECTOR, gate_attrs, entry->ist);
	}

	idtp.base = (u64)&idt[0];
	idtp.limit = sizeof(idt) - 1;

	idt_load(&idtp);
	pr_info("IDT loaded and ready.\n");
}