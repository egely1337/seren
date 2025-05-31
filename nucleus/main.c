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
#include <stddef.h>

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

static void test_keyboard_handler(irq_context_t *context
                                  __attribute__((unused))) {
    uint8_t scancode = inb(0x60);
    printk(KERN_INFO "Keyboard press! Vector: %lu, Scancode: 0x%x\n",
           context->vector_number, scancode);
}

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

            char *my_string = (char *)kmalloc(100);
            if (my_string) {
                strcpy(my_string, "Hello from kheap!");
                printk(KERN_INFO "kmalloc test: %s\n", my_string);
                kfree(my_string);
            } else {
                printk(KERN_WARN "kmalloc test failed!\n");
            }
        } else {
            printk(KERN_EMERG "Failed to initialize kernel heap!\n");
        }
    } else {
        printk(KERN_EMERG "Failed to allocate pages for kernel heap!\n");
    }

    interrupt_register_irq_handler(1, test_keyboard_handler);
    printk(KERN_INFO "Registered example Timer and Keyboard IRQ handlers.\n");

    pic_unmask_irq(1);
    printk(KERN_INFO "Unmasked Keyboard (IRQ1).\n");

    printk(KERN_INFO "Enabling interrupts (STI).\n");
    __asm__ volatile("sti");

    printk(
        KERN_INFO
        "Initialization sequence complete. Entering halt loop. See you <3\n");

    goto halt_loop;

halt_loop:
    while (1) {
        __asm__ volatile("hlt");
    }
}