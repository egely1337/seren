#include <drivers/pit.h>
#include <io.h>
#include <nucleus/interrupt.h>
#include <nucleus/printk.h>
#include <pic.h>

#define PIT_PFX "pit: "

#define pit_info(fmt, ...) pr_info(PIT_PFX fmt, ##__VA_ARGS__)

static volatile u64 g_system_ticks = 0;

void timer_handler(void) {
    g_system_ticks++;
    pic_send_eoi(0);
}

void timer_init(void) {
    u32 frequency = 100; // 100 Hz
    u16 divisor = TIMER_FREQUENCY / frequency;

    outb(0x43, 0x36);

    outb(0x40, (u8)divisor & 0xFF);
    outb(0x40, (u8)(divisor >> 8) & 0xFF);

    interrupt_register_irq_handler(TIMER_IRQ, (irq_c_handler_t)timer_handler);
    pit_info("initialized with %u Hz frequency\n", frequency);
}

u64 timer_get_uptime_ms(void) { return g_system_ticks * 10; }
