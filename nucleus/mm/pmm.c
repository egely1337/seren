#include <limine.h>
#include <nucleus/config.h>
#include <nucleus/mm/pmm.h>
#include <nucleus/panic.h>
#include <nucleus/printk.h>
#include <nucleus/types.h>

#define PMM_PFX "pmm: "

#define pmm_info(fmt, ...) pr_info(PMM_PFX fmt, ##__VA_ARGS__)
#define pmm_warn(fmt, ...) pr_warn(PMM_PFX fmt, ##__VA_ARGS__)
#define pmm_dbg(fmt, ...)  pr_debug(PMM_PFX fmt, ##__VA_ARGS__)

// --- PMM Internal State ---
static uint8_t *page_bitmap = NULL;
static uint64_t total_pages_managed = 0;
static uint64_t highest_phys_addr_managed = 0;
static uint64_t free_pages_count = 0;
static uint64_t used_pages_count = 0;

extern volatile struct limine_hhdm_request hhdm_request;

// Linker-defined symbol for the end of the kernel image (VMA)
extern char _kernel_end[];

/**
 * @brief Marks a page as used in the bitmap (sets the bit).
 * @param page_index The index of the page to mark.
 */
static void bitmap_set_page(uint64_t page_index) {
    if (!page_bitmap || page_index >= total_pages_managed)
        return;
    uint64_t byte_index = page_index / 8;
    uint8_t bit_index = page_index % 8;
    page_bitmap[byte_index] |= (1 << bit_index);
}

/**
 * @brief Marks a page as free in the bitmap (clears the bit).
 * @param page_index The index of the page to mark.
 */
static void bitmap_clear_page(uint64_t page_index) {
    if (!page_bitmap || page_index >= total_pages_managed)
        return;
    uint64_t byte_index = page_index / 8;
    uint8_t bit_index = page_index % 8;
    page_bitmap[byte_index] &= ~(1 << bit_index);
}

/**
 * @brief Checks if a page is marked as used in the bitmap.
 * @param page_index The index of the page to test.
 * @return 1 if the page is used (bit is set), 0 if free (bit is clear).
 * Returns 1 (used) if page_index is out of bounds, as safety measure.
 */
static int bitmap_is_page_used(uint64_t page_index) {
    if (page_index >= total_pages_managed)
        return 1;
    uint64_t byte_index = page_index / 8;
    uint8_t bit_index = page_index % 8;
    return (page_bitmap[byte_index] & (1 << bit_index)) != 0;
}

/**
 * @brief Finds the first block of `num_pages` consecutive free pages.
 * @param num_pages The number of contiguous free pages to find.
 * @return The starting page index of the free block, or -1 if no such block is
 * found.
 */
static int64_t bitmap_find_first_free_block(size_t num_pages) {
    if (num_pages == 0 || !page_bitmap)
        return -1;
    uint64_t consecutive_free_count = 0;
    for (uint64_t i = 0; i < total_pages_managed; i++) {
        if (!bitmap_is_page_used(i)) {
            consecutive_free_count++;
            if (consecutive_free_count == num_pages) {
                return i - num_pages + 1;
            }
        } else {
            consecutive_free_count = 0;
        }
    }
    return -1;
}

// --- PMM Public API---

/**
 * @brief Initializes the Physical Memory Manager.
 * This is the big one. It figures out available memory, sets up our tracking
 * bitmap, and reserves space for the kernel and the bitmap itself.
 */
void pmm_init(volatile struct limine_memmap_request *memmap_request) {
    if (!memmap_request || !memmap_request->response) {
        panic(PMM_PFX "Limine memmap request or response is NULL!", NULL);
    }
    if (!hhdm_request.response) {
        panic(PMM_PFX "Limine HHDM response is NULL!", NULL);
    }

    struct limine_memmap_response *memmap = memmap_request->response;
    uint64_t hhdm_offset = hhdm_request.response->offset;

    pmm_dbg("HHDM virtual offset: 0x%lx\n", hhdm_offset);
    pmm_dbg("Limine reported %lu memory map entries.\n", memmap->entry_count);

    // Step 1: Determine the highest physical address to manage.
    highest_phys_addr_managed = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE ||
            entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            if (entry->base + entry->length > highest_phys_addr_managed) {
                highest_phys_addr_managed = entry->base + entry->length;
            }
        }
    }
    if (highest_phys_addr_managed == 0) {
        panic(PMM_PFX "no usable or bootloader-reclaimable memory found.",
              NULL);
    }
    highest_phys_addr_managed =
        (highest_phys_addr_managed + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    total_pages_managed = highest_phys_addr_managed / PAGE_SIZE;
    if (total_pages_managed == 0 && highest_phys_addr_managed > 0)
        total_pages_managed = 1;

    pmm_dbg("highest physical address found: 0x%lx\n",
            highest_phys_addr_managed);
    pmm_dbg("total pages to manage: %lu\n", total_pages_managed);

    // Step 2: Determine where our kernel image ends in physical memory.
    uint64_t kernel_phys_start = KERNEL_PHYSICAL_LOAD_ADDR;
    uint64_t kernel_phys_end = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            if (entry->base <= kernel_phys_start &&
                (entry->base + entry->length) > kernel_phys_start) {
                kernel_phys_end = entry->base + entry->length;
                pmm_dbg("kernel region from memmap: base=0x%lx, length=0x%lx\n",
                        entry->base, entry->length);
                break;
            }
        }
    }
    if (kernel_phys_end == 0) {
        uintptr_t kernel_virtual_end_addr = (uintptr_t)_kernel_end;
        kernel_phys_end = KERNEL_PHYSICAL_LOAD_ADDR +
                          (kernel_virtual_end_addr - KERNEL_VIRTUAL_BASE);
        pmm_warn("could not find definitive kernel region in memmap, "
                 "calculating from linker symbols.\n");
    }
    kernel_phys_end = (kernel_phys_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    pmm_dbg("kernel physical footprint ends at (aligned): 0x%lx\n",
            kernel_phys_end);

    // Step 3: Calculate bitmap size and find a spot for it.
    size_t bitmap_size_bytes = (total_pages_managed + 7) / 8;
    pmm_dbg("bitmap requires %u bytes (%u KiB)\n", bitmap_size_bytes,
            bitmap_size_bytes / 1024);
    uint64_t bitmap_physical_addr = 0;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t current_region_start = entry->base;
            uint64_t current_region_end = entry->base + entry->length;
            uint64_t potential_start = (kernel_phys_end > current_region_start)
                                           ? kernel_phys_end
                                           : current_region_start;
            potential_start =
                (potential_start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

            if (potential_start < current_region_end &&
                (current_region_end - potential_start) >= bitmap_size_bytes) {
                bitmap_physical_addr = potential_start;
                break;
            }
        }
    }
    if (bitmap_physical_addr == 0) {
        panic(PMM_PFX "failed to find a suitable memory region for the bitmap!",
              NULL);
    }
    page_bitmap = (uint8_t *)(hhdm_offset + bitmap_physical_addr);
    pmm_info("bitmap placed at physical 0x%lx (virtual 0x%p)\n",
             (void *)bitmap_physical_addr, page_bitmap);

    // Step 4: Initialize bitmap and page counts
    for (size_t k = 0; k < bitmap_size_bytes; k++) {
        page_bitmap[k] = 0x00;
    }

    for (uint64_t page_idx = 0; page_idx < total_pages_managed; page_idx++) {
        uint64_t current_phys_addr = page_idx * PAGE_SIZE;
        int is_usable_or_reclaimable = 0;
        for (uint64_t k = 0; k < memmap->entry_count; k++) {
            struct limine_memmap_entry *entry = memmap->entries[k];
            if (current_phys_addr >= entry->base &&
                current_phys_addr < (entry->base + entry->length)) {
                if (entry->type == LIMINE_MEMMAP_USABLE ||
                    entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
                    is_usable_or_reclaimable = 1;
                }
                break; // Found the entry covering this page
            }
        }
        if (!is_usable_or_reclaimable) {
            bitmap_set_page(page_idx); // Mark as used if not usable/reclaimable
        }
    }

    uint64_t kernel_start_page_idx = kernel_phys_start / PAGE_SIZE;
    uint64_t kernel_num_pages =
        (kernel_phys_end - kernel_phys_start) / PAGE_SIZE;
    for (uint64_t k = 0; k < kernel_num_pages; k++) {
        bitmap_set_page(kernel_start_page_idx + k);
    }
    pmm_dbg("reserved %lu pages for the kernel.\n", kernel_num_pages);

    uint64_t bitmap_start_page_idx = bitmap_physical_addr / PAGE_SIZE;
    uint64_t bitmap_num_pages_val =
        (bitmap_size_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint64_t k = 0; k < bitmap_num_pages_val; k++) {
        bitmap_set_page(bitmap_start_page_idx + k);
    }
    pmm_dbg("reserved %lu pages for the PMM bitmap itself.\n",
            bitmap_num_pages_val);

    free_pages_count = 0;
    used_pages_count = 0;
    for (uint64_t i = 0; i < total_pages_managed; i++) {
        if (bitmap_is_page_used(i)) {
            used_pages_count++;
        } else {
            free_pages_count++;
        }
    }

    pmm_info("initialization complete.\n");
    pmm_info("total RAM: %lu MiB, usable: %lu MiB, used: %lu KiB\n",
             (total_pages_managed * PAGE_SIZE) / (1024 * 1024),
             (free_pages_count * PAGE_SIZE) / (1024 * 1024),
             (used_pages_count * PAGE_SIZE) / 1024);
}

// --- Allocation and Deallocation Functions ---

void *pmm_alloc_page(void) { return pmm_alloc_contiguous_pages(1); }

void *pmm_alloc_contiguous_pages(size_t num_pages) {
    // TODO: Add locking for SMP safety

    if (page_bitmap == NULL || num_pages == 0) {
        pmm_warn(
            "alloc failed: PMM not initialized or zero pages requested.\n");
        return NULL;
    }

    if (free_pages_count < num_pages) {
        pmm_warn("alloc failed: not enough free memory (requested %lu pages, "
                 "%lu available).\n",
                 num_pages, free_pages_count);
        return NULL;
    }

    int64_t start_page_index = bitmap_find_first_free_block(num_pages);

    if (start_page_index == -1) {
        pmm_warn(
            "alloc failed: could not find a contiguous block of %lu pages.\n",
            num_pages);
        return NULL;
    }

    // Mark pages as used in bitmap and update counts
    for (size_t i = 0; i < num_pages; i++) {
        bitmap_set_page(start_page_index + i);
    }

    free_pages_count -= num_pages;
    used_pages_count += num_pages;

    void *allocated_address = (void *)((uint64_t)start_page_index * PAGE_SIZE);
    pmm_dbg("allocated %lu pages at physical address %p\n", num_pages,
            allocated_address);

    return allocated_address;
}

void pmm_free_page(void *p_addr) { pmm_free_contiguous_pages(p_addr, 1); }

void pmm_free_contiguous_pages(void *p_addr, size_t num_pages) {
    if (page_bitmap == NULL || p_addr == NULL || num_pages == 0) {
        return;
    }

    uint64_t physical_address = (uint64_t)p_addr;
    if (physical_address % PAGE_SIZE != 0) {
        pmm_warn("attempted to free unaligned physical address %p\n", p_addr);
        return;
    }

    uint64_t start_page_index = physical_address / PAGE_SIZE;

    // TODO: Add locking for SMP safety

    for (size_t i = 0; i < num_pages; i++) {
        uint64_t current_page_index = start_page_index + i;
        if (current_page_index >= total_pages_managed) {
            pmm_warn("attempted to free page index %lu which is out of bounds "
                     "(%lu).\n",
                     current_page_index, total_pages_managed);
            break;
        }
        if (!bitmap_is_page_used(current_page_index)) {
            pmm_warn(
                "double free detected for page at physical address 0x%lx\n",
                (uint64_t)current_page_index * PAGE_SIZE);
            return;
        } else {
            bitmap_clear_page(current_page_index);

            free_pages_count++;
            used_pages_count--;
        }
    }
}

// --- Getter Functions ---

uint64_t pmm_get_total_memory(void) { return total_pages_managed * PAGE_SIZE; }

uint64_t pmm_get_free_memory(void) { return free_pages_count * PAGE_SIZE; }

uint64_t pmm_get_used_memory(void) { return used_pages_count * PAGE_SIZE; }