#include <idt.h>
#include <nucleus/printk.h>
#include <pic.h>

#define ARCH_PFX "x86_64: "

#define arch_info(fmt, ...) pr_info(ARCH_PFX fmt, ##__VA_ARGS__)

void arch_init(void) {
    arch_info("initializing IDT.\n");
    idt_init();

    arch_info("initializing and remapping legacy PIC...\n");
    pic_remap_and_init();
}