#include <drivers/timer.h>
#include <nucleus/interrupt.h>
#include <io.h>
#include <nucleus/printk.h>

void timer_handler(void) {
    pr_info("hello world\n");
}

void timer_init(void) {
    /* Initialize timer here */
    interrupt_register_irq_handler(TIMER_IRQ, (irq_c_handler_t)timer_handler);

    uint16_t divisior = TIMER_FREQUENCY / 100;
    outb(0x43, 0x36);
    outb(0x40, (uint16_t)divisior & 0xFFFF);
    outb(0x40, (uint16_t)(divisior >> 16));
}