#include <idt.h>
#include <nucleus/printk.h>
#include <pic.h>

void arch_init(void) {
    idt_init();
    pr_info("x86_64: IDT initialized.\n");

    pic_remap_and_init();
    pr_info("x86_64: PIC remapped and initialized.\n");
}