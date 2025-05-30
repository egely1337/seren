#include <limine.h>
#include <nucleus/console.h>
#include <nucleus/idt.h>
#include <nucleus/printk.h>
#include <nucleus/mem/pmm.h>
#include <stddef.h>

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

void run_pmm_tests() {
    printk(KERN_INFO "--- Starting Intensive PMM Tests ---\n");

    uint64_t initial_free_pages = pmm_get_free_memory() / PAGE_SIZE;
    uint64_t initial_used_pages = pmm_get_used_memory() / PAGE_SIZE;
    printk(KERN_DEBUG "PMM_Test: Initial state - Free: %lu pages, Used: %lu pages\n", initial_free_pages, initial_used_pages);

    #define MAX_TOTAL_TEST_ALLOCATIONS 200
    void *allocations[MAX_TOTAL_TEST_ALLOCATIONS];
    size_t allocation_sizes[MAX_TOTAL_TEST_ALLOCATIONS];
    int alloc_idx = 0;

    // Test 1: Allocate some single pages
    printk(KERN_INFO "PMM_Test: Allocating some single pages...\n");
    int single_alloc_target = 10;
    if (single_alloc_target > MAX_TOTAL_TEST_ALLOCATIONS / 2) {
        single_alloc_target = MAX_TOTAL_TEST_ALLOCATIONS / 2;
    }

    for (int i = 0; i < single_alloc_target; i++) {
        if (alloc_idx >= MAX_TOTAL_TEST_ALLOCATIONS) {
            printk(KERN_WARN "PMM_Test: Reached max allocations tracking array limit early.\n");
            break;
        }
        if (pmm_get_free_memory() < PAGE_SIZE * 10) {
             printk(KERN_WARN "PMM_Test: Low memory, stopping single page allocations.\n");
             break;
        }
        allocations[alloc_idx] = pmm_alloc_page();
        if (allocations[alloc_idx]) {
            allocation_sizes[alloc_idx] = 1;
            alloc_idx++;
        } else {
            printk(KERN_WARN "PMM_Test: Single page allocation failed at iteration %d. Free pages: %lu\n", i, pmm_get_free_memory() / PAGE_SIZE);
            break;
        }
    }
    printk(KERN_INFO "PMM_Test: Allocated %d single pages (tracked index up to %d).\n", single_alloc_target, alloc_idx);
    printk(KERN_DEBUG "PMM_Test: After single allocs - Free: %lu pages, Used: %lu pages\n", pmm_get_free_memory() / PAGE_SIZE, pmm_get_used_memory() / PAGE_SIZE);

    // Test 2: Allocate contiguous blocks
    printk(KERN_INFO "PMM_Test: Allocating contiguous blocks...\n");
    size_t block_sizes[] = {1, 5, 16, 30, 60, 100};
    int blocks_allocated_count = 0;

    for (size_t i = 0; i < sizeof(block_sizes) / sizeof(block_sizes[0]); i++) {
        if (alloc_idx >= MAX_TOTAL_TEST_ALLOCATIONS) {
             printk(KERN_WARN "PMM_Test: Reached max allocations tracking array limit for blocks.\n");
             break;
        }

        if (pmm_get_free_memory() < (block_sizes[i] * PAGE_SIZE) + (PAGE_SIZE * 5)) { 
            printk(KERN_WARN "PMM_Test: Not enough space for block of %u pages. Skipping.\n", block_sizes[i]);
            continue;
        }
        allocations[alloc_idx] = pmm_alloc_contiguous_pages(block_sizes[i]);
        if (allocations[alloc_idx]) {
            allocation_sizes[alloc_idx] = block_sizes[i];
            printk(KERN_DEBUG "PMM_Test: Allocated block of %u pages at %p (idx %d)\n", block_sizes[i], allocations[alloc_idx], alloc_idx);
            alloc_idx++;
            blocks_allocated_count++;
        } else {
            printk(KERN_WARN "PMM_Test: Failed to allocate block of %u pages. Free pages: %lu\n", block_sizes[i], pmm_get_free_memory() / PAGE_SIZE);
        }
    }
    printk(KERN_INFO "PMM_Test: Attempted to allocate %d blocks.\n", blocks_allocated_count);
    printk(KERN_DEBUG "PMM_Test: After block allocs - Free: %lu pages, Used: %lu pages\n", pmm_get_free_memory() / PAGE_SIZE, pmm_get_used_memory() / PAGE_SIZE);

    // Test 3: Free all allocated pages (in reverse order of allocation)
    printk(KERN_INFO "PMM_Test: Freeing all %d tracked allocations (in reverse order)...\n", alloc_idx);
    for (int i = alloc_idx - 1; i >= 0; i--) {
        if (allocations[i]) {

            pmm_free_contiguous_pages(allocations[i], allocation_sizes[i]);
            allocations[i] = NULL; 
        }
    }

    alloc_idx = 0;


    printk(KERN_INFO "PMM_Test: After freeing all pages (hopefully!).\n");
    printk(KERN_INFO "PMM_Test: Final state - Free: %lu pages, Used: %lu pages\n", pmm_get_free_memory() / PAGE_SIZE, pmm_get_used_memory() / PAGE_SIZE);

    if (pmm_get_free_memory() / PAGE_SIZE == initial_free_pages) {
        printk(KERN_INFO "PMM_Test: SUCCESS - Free page count matches initial state.\n");
    } else {
        printk(KERN_WARN "PMM_Test: WARNING - Free page count (%lu) does not match initial state (%lu).\n",
               pmm_get_free_memory() / PAGE_SIZE, initial_free_pages);
    }

    // Test 4: Attempt to allocate all remaining free memory
    printk(KERN_INFO "PMM_Test: Attempting to allocate all remaining free memory...\n");
    uint64_t pages_to_try_alloc = pmm_get_free_memory() / PAGE_SIZE;
    uint64_t successfully_allocated_fill_test = 0;
    void *temp_allocs_for_fill[pages_to_try_alloc > MAX_TOTAL_TEST_ALLOCATIONS ? MAX_TOTAL_TEST_ALLOCATIONS : (pages_to_try_alloc == 0 ? 1 : pages_to_try_alloc)]; // Dynamic VLA or fixed max
    
    uint64_t alloc_limit_for_fill = pages_to_try_alloc;
    if (pages_to_try_alloc > MAX_TOTAL_TEST_ALLOCATIONS) {
        printk(KERN_WARN "PMM_Test: Too many pages to track for fill test (%lu), limiting to %d.\n", pages_to_try_alloc, MAX_TOTAL_TEST_ALLOCATIONS);
        alloc_limit_for_fill = MAX_TOTAL_TEST_ALLOCATIONS;
    }
    if (pages_to_try_alloc == 0 && pmm_get_free_memory() > 0) {
         printk(KERN_WARN "PMM_Test: Less than a full page of free memory, cannot run fill test for pages.\n");
    }


    for (uint64_t i = 0; i < alloc_limit_for_fill; i++) {
        temp_allocs_for_fill[i] = pmm_alloc_page();
        if (temp_allocs_for_fill[i]) {
            successfully_allocated_fill_test++;
        } else {
            printk(KERN_WARN "PMM_Test: Fill test failed to allocate page #%lu. Free pages reported: %lu\n", i + 1, pmm_get_free_memory() / PAGE_SIZE);
            break;
        }
    }
    printk(KERN_INFO "PMM_Test: Fill test allocated %lu pages.\n", successfully_allocated_fill_test);
    printk(KERN_DEBUG "PMM_Test: After fill test - Free: %lu pages, Used: %lu pages\n", pmm_get_free_memory() / PAGE_SIZE, pmm_get_used_memory() / PAGE_SIZE);

    printk(KERN_INFO "PMM_Test: Freeing pages from fill test...\n");
    for (uint64_t i = 0; i < successfully_allocated_fill_test; i++) {
        if (temp_allocs_for_fill[i]) {
            pmm_free_page(temp_allocs_for_fill[i]);
        }
    }
    printk(KERN_INFO "PMM_Test: After freeing fill test pages - Free: %lu pages, Used: %lu pages\n", pmm_get_free_memory() / PAGE_SIZE, pmm_get_used_memory() / PAGE_SIZE);
    if (pmm_get_free_memory() / PAGE_SIZE == initial_free_pages) {
        printk(KERN_INFO "PMM_Test: SUCCESS - Free page count restored after fill test.\n");
    } else {
        printk(KERN_WARN "PMM_Test: WARNING - Free page count (%lu) not restored after fill test (%lu).\n",
               pmm_get_free_memory() / PAGE_SIZE, initial_free_pages);
    }

    printk(KERN_INFO "--- Intensive PMM Tests Finished ---\n");
}

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

    if (!hhdm_request.response) {
        printk(KERN_EMERG "CRITICAL: Limine HHDM not available! Halting.\n");
        goto halt_loop;
    }

    if (!memmap_request.response) {
        printk(KERN_EMERG "CRITICAL: Limine MEMMAP not available! Halting.\n");
        goto halt_loop;
    }

    printk(KERN_INFO "Limine memmap and HHDM responses available.\n");

    pmm_init(&memmap_request);

    run_pmm_tests();

    printk(KERN_INFO "Initialization sequence complete. Entering halt loop. See you <3\n");

    goto halt_loop;

halt_loop:
    while(1) {
        __asm__ volatile("hlt");
    }
}