#include <arch.h>
#include <asm/irqflags.h>
#include <drivers/input/keyboard.h>
#include <drivers/pit.h>
#include <limine.h>
#include <nucleus/interrupt.h>
#include <nucleus/mm/pmm.h>
#include <nucleus/mm/slab.h>
#include <nucleus/printk.h>
#include <nucleus/sched/sched.h>
#include <nucleus/tty/console.h>
#include <nucleus/types.h>
#include <lib/string.h>
#include <pic.h>

__attribute__((
    used,
    section(".limine_requests"))) volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

__attribute__((used,
               section(".limine_requests"))) volatile struct limine_hhdm_request
    hhdm_request = {.id = LIMINE_HHDM_REQUEST, .revision = 0};

__attribute__((
    used, section(".limine_requests"))) volatile struct limine_memmap_request
    memmap_request = {.id = LIMINE_MEMMAP_REQUEST, .revision = 0};

void kmain(void) {
    console_init();
    pr_info("Seren OS booting...\n");
    pr_info("LFB GFX, PSF Font, console initialized.\n");

    arch_init();

    mem_init(&memmap_request);
    kheap_init(NULL, 0);

    keyboard_init();
    pr_info("Keyboard driver initialized.\n");

    timer_init();
    pr_info("Timer driver initialized.\n");

    // TODO: Move this behind an arch-independent API
    pic_unmask_irq(1);
    pr_info("Unmasked Keyboard (IRQ1).\n");

    pic_unmask_irq(0);
    pr_info("Unmasked Timer (IRQ0).\n");

    sched_init();

    local_irq_enable();

    pr_info("Initialization sequence complete. You can now type. See you <3\n");

    while (1) {
        char c = keyboard_getchar();
        printk("%c", c);
    }
}