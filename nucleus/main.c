#include <limine.h>
#include <nucleus/console.h>
#include <nucleus/idt.h>
#include <nucleus/printk.h>
#include <stddef.h>

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void kmain(void) {
    console_init();
    printk(KERN_INFO "Seren OS - Nucleus Kernel Booting...\n");
    printk(KERN_INFO "LFB GFX, PSF Font, Console Initialized.\n");

    idt_init();
    printk(KERN_INFO "IDT Initialized and Loaded.\n");

    if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0) {
        struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
        if (fb && fb->address) {
            printk(KERN_DEBUG "Framebuffer address: %p, %ux%u %ubpp, pitch %u\n",
                   fb->address, fb->width, fb->height, fb->bpp, fb->pitch);
        }
    } else {
        printk(KERN_WARN "No framebuffer response from Limine in kmain.\n");
    }

    printk("Attempting division by zero...\n");

    volatile int divisor = 0;
    int result;

    result = 5 / divisor;

    printk("This line should NOT be reached if divide by zero exception worked!\n");

    if (result == 0) {
        printk("Result was zero (but still shouldn't be here).\n");
    }

    goto halt_kernel_loop;

halt_kernel_loop:
    while(1) {
        __asm__ volatile("hlt");
    }
}