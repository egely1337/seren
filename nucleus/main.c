#include <limine.h>
#include <nucleus/console.h>
#include <stddef.h>

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void kmain(void) {
    console_init();

    console_writestring("Hello, SerenOS!");

    goto halt_kernel_loop;

halt_kernel_loop:
    while(1) {
        __asm__ volatile("hlt");
    }
}