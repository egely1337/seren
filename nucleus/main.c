#include <limine.h>
#include <nucleus/console.h>
#include <nucleus/idt.h>
#include <stddef.h>

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void kmain(void) {
    idt_init();

    console_init();

    console_writestring("Attempting division by zero...\n");

    volatile int divisor = 0;
    int result;

    result = 5 / divisor;

    console_writestring("This line should NOT be reached if divide by zero exception worked!\n");
    if (result == 0) {
        console_writestring("Result was zero (but still shouldn't be here).\n");
    }

    goto halt_kernel_loop;

halt_kernel_loop:
    while(1) {
        __asm__ volatile("hlt");
    }
}