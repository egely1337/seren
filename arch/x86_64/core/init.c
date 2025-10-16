#define pr_fmt(fmt) "x86_64: " fmt

#include <idt.h>
#include <nucleus/printk.h>
#include <pic.h>

void arch_init(void) {
    pr_info("initializing IDT.\n");
    idt_init();

    pr_info("initializing and remapping legacy PIC...\n");
    pic_remap_and_init();
}