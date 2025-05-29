#include <limine.h>
#include <stddef.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void kmain(void) {
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        goto halt_kernel_loop;
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    uint32_t white_color = 0x00FFFFFF;

    if (fb->address == NULL) {
        goto halt_kernel_loop;
    }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;

    for (uint64_t y = 0; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            if (x % 100 == 0 || y % 100 == 0) {
                fb_ptr[y * (fb->pitch / 4) + x] = white_color;
            }
        }
    }

halt_kernel_loop:
    while(1) {
        __asm__ volatile("hlt");
    }
}