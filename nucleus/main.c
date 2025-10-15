#include <drivers/keyboard.h>
#include <drivers/pic.h>
#include <limine.h>
#include <nucleus/console.h>
#include <nucleus/idt.h>
#include <nucleus/interrupt.h>
#include <nucleus/io.h>
#include <nucleus/memory/kheap.h>
#include <nucleus/memory/pmm.h>
#include <nucleus/printk.h>
#include <nucleus/string.h>
#include <nucleus/types.h>

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
    printk(KERN_INFO "Seren OS - Nucleus Kernel Booting...\n");
    printk(KERN_INFO "LFB GFX, PSF Font, Console Initialized.\n");

    idt_init();
    printk(KERN_INFO "IDT Initialized and Loaded.\n");

    pic_remap_and_init();
    printk(KERN_INFO "PICs remapped and initialized.\n");

    pmm_init(&memmap_request);

#define KHEAP_INITIAL_PAGES 4
    void *heap_start_phys = pmm_alloc_contiguous_pages(KHEAP_INITIAL_PAGES);
    if (heap_start_phys) {
        void *heap_start_virt = (void *)(hhdm_request.response->offset +
                                         (uintptr_t)heap_start_phys);

        if (kheap_init(heap_start_virt, KHEAP_INITIAL_PAGES * PAGE_SIZE) == 0) {
            printk(KERN_INFO "Kernel heap initialized successfully.\n");
        } else {
            printk(KERN_EMERG "Failed to initialize kernel heap!\n");
        }
    } else {
        printk(KERN_EMERG "Failed to allocate pages for kernel heap!\n");
    }

    printk(KERN_INFO "Registered example Timer and Keyboard IRQ handlers.\n");

    keyboard_init();
    printk(KERN_INFO "Keyboard driver initialized.\n");

    pic_unmask_irq(1);
    printk(KERN_INFO "Unmasked Keyboard (IRQ1).\n");

    printk(KERN_INFO "Enabling interrupts (STI).\n");
    __asm__ volatile("sti");

    printk(KERN_INFO
           "Initialization sequence complete. You can now type. See you <3\n");

    while (1) {
        char c = keyboard_getchar();
        console_putchar(c);
    }

    goto halt_loop;

halt_loop:
    while (1) {
        __asm__ volatile("hlt");
    }
}