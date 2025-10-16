#include <idt.h>
#include <nucleus/printk.h>
#include <nucleus/types.h>
#include <pic.h>

#define IDT_PFX           "idt: "
#define idt_dbg(fmt, ...) pr_debug(IDT_PFX fmt, ##__VA_ARGS__)

static idt_entry_t idt[IDT_MAX_DESCRIPTORS];
static idt_ptr_t idtp;

#define KERNEL_CS 0x28

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
    idtp.base = (u64)&idt[0];
    idtp.limit = (u16)(sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1);

    idt_dbg("table cleared. base: 0x%p, limit: 0x%x\n", (void *)idtp.base,
            idtp.limit);

    for (int i = 0; i < IDT_MAX_DESCRIPTORS; i++) {
        idt_set_gate(i, 0, 0, 0, 0);
    }

    u8 gate_attributes =
        IDT_ATTR_PRESENT | IDT_TYPE_INTERRUPT_GATE | IDT_ATTR_RING0;
    u8 irq_gate_attributes =
        IDT_ATTR_PRESENT | IDT_TYPE_INTERRUPT_GATE | IDT_ATTR_RING0;

    idt_set_gate(0, (u64)isr0, KERNEL_CS, gate_attributes,
                 0);                                           // Divide by Zero
    idt_set_gate(1, (u64)isr1, KERNEL_CS, gate_attributes, 0); // Debug
    idt_set_gate(2, (u64)isr2, KERNEL_CS, gate_attributes, 0); // NMI
    idt_set_gate(3, (u64)isr3, KERNEL_CS, gate_attributes,
                 0);                                           // Breakpoint
    idt_set_gate(4, (u64)isr4, KERNEL_CS, gate_attributes, 0); // Overflow
    idt_set_gate(5, (u64)isr5, KERNEL_CS, gate_attributes,
                 0); // Bound Range Exceeded
    idt_set_gate(6, (u64)isr6, KERNEL_CS, gate_attributes,
                 0); // Invalid Opcode
    idt_set_gate(7, (u64)isr7, KERNEL_CS, gate_attributes,
                 0); // Device Not Available
    idt_set_gate(8, (u64)isr8, KERNEL_CS, gate_attributes,
                 0); // Double Fault (Consider using IST for this one)
    idt_set_gate(9, (u64)isr9, KERNEL_CS, gate_attributes,
                 0); // Coprocessor Segment Overrun
    idt_set_gate(10, (u64)isr10, KERNEL_CS, gate_attributes,
                 0); // Invalid TSS
    idt_set_gate(11, (u64)isr11, KERNEL_CS, gate_attributes,
                 0); // Segment Not Present
    idt_set_gate(12, (u64)isr12, KERNEL_CS, gate_attributes,
                 0); // Stack-Segment Fault
    idt_set_gate(13, (u64)isr13, KERNEL_CS, gate_attributes,
                 0); // General Protection Fault
    idt_set_gate(14, (u64)isr14, KERNEL_CS, gate_attributes,
                 0); // Page Fault
    // ISR 15 is reserved
    idt_set_gate(16, (u64)isr16, KERNEL_CS, gate_attributes,
                 0); // x87 Floating-Point
    idt_set_gate(17, (u64)isr17, KERNEL_CS, gate_attributes,
                 0); // Alignment Check
    idt_set_gate(18, (u64)isr18, KERNEL_CS, gate_attributes,
                 0); // Machine Check
    idt_set_gate(19, (u64)isr19, KERNEL_CS, gate_attributes,
                 0); // SIMD Floating-Point
    idt_set_gate(20, (u64)isr20, KERNEL_CS, gate_attributes,
                 0); // Virtualization

    idt_set_gate(PIC_IRQ_OFFSET_MASTER + 0, (u64)irq_stub_0, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_MASTER + 1, (u64)irq_stub_1, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_MASTER + 2, (u64)irq_stub_2, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_MASTER + 3, (u64)irq_stub_3, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_MASTER + 4, (u64)irq_stub_4, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_MASTER + 5, (u64)irq_stub_5, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_MASTER + 6, (u64)irq_stub_6, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_MASTER + 7, (u64)irq_stub_7, KERNEL_CS,
                 irq_gate_attributes, 0);

    idt_set_gate(PIC_IRQ_OFFSET_SLAVE + 0, (u64)irq_stub_8, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_SLAVE + 1, (u64)irq_stub_9, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_SLAVE + 2, (u64)irq_stub_10, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_SLAVE + 3, (u64)irq_stub_11, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_SLAVE + 4, (u64)irq_stub_12, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_SLAVE + 5, (u64)irq_stub_13, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_SLAVE + 6, (u64)irq_stub_14, KERNEL_CS,
                 irq_gate_attributes, 0);
    idt_set_gate(PIC_IRQ_OFFSET_SLAVE + 7, (u64)irq_stub_15, KERNEL_CS,
                 irq_gate_attributes, 0);

    idt_load(&idtp);
    idt_dbg("IDT loaded.\n");
}
