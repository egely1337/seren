#include <idt.h>
#include <nucleus/printk.h>
#include <pic.h>

void arch_init(void) {
    idt_init();
    printk(KERN_INFO "x86_64: IDT initialized.\n");

    pic_remap_and_init();
    printk(KERN_INFO "x86_64: PIC remapped and initialized.\n");
}