// SPDX-License-Identifier: Apache-2.0

#ifndef _NUCLEUS_MM_PMM_H
#define _NUCLEUS_MM_PMM_H

#include <limine.h>
#include <nucleus/stddef.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE  (1UL << PAGE_SHIFT)
#define PAGE_MASK  (~(PAGE_SIZE - 1))

#define PFN_ALIGN(x) (((unsigned long)(x) + PAGE_SIZE - 1) & PAGE_MASK)
#define PFN_UP(x)    (((unsigned_long)(x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DOWN(x)  ((unsigned long)(x) >> PAGE_SHIFT)
#define PFN_PHYS(x)  ((phys_addr_t)(x) << PAGE_SHIFT)

/**
 * struct page - Abstract handle for a physical page frame
 * @pfn:    Page frame number
 */
struct page {
	u64 pfn;
};

/**
 * mem_init - Initialize the physical memory manager
 * @memmap_request: Pointer to the Limine memmap request
 */
void mem_init(volatile struct limine_memmap_request *memmap_request);

/**
 * alloc_pages - Allocate a contiguous block of physical pages
 * @order:  The order of the allocation (2^order pages)
 */
struct page *alloc_pages(u32 order);

/**
 * free_pages - Free a contiguous block of physical pages
 * @page:   Pointer to the first `struct page` in the block to be freed.
 * @order:  The order of the block that was allocated.
 */
void free_pages(struct page *page, u32 order);

/**
 * alloc_page - Allocate a single physical page
 *
 * Inline wrapper around alloc_pages(0);
 */
static inline struct page *alloc_page(void) { return alloc_pages(0); }

/**
 * free_page - Free a single physical page
 * @page:   Pointer to the `struct page` to be freed
 */
static inline void free_page(struct page *page) { free_pages(page, 0); }

/**
 * page_to_phys - Convert a `struct page` to its physical address
 * @page:   The page structure to convert.
 */
phys_addr_t page_to_phys(struct page *page);

/**
 * phys_to_page - Convert a physical address to its `struct page`
 * @phys:   The physical memory address.
 */
struct page *phys_to_page(phys_addr_t phys);

/**
 * virt_to_page - Convert a kernel virtual address to its `struct page`
 * @addr:   The kernel virtual address (from the HHDM)
 */
static inline struct page *virt_to_page(void *addr) {
	extern volatile struct limine_hhdm_request hhdm_request;
	u64 hhdm_offset = hhdm_request.response->offset;
	phys_addr_t phys = (phys_addr_t)addr - hhdm_offset;
	return phys_to_page(phys);
}

/**
 * page_to_virt - Convert a `struct page` to its kernel virtual address
 * @addr:   The page structure to convert
 */
static inline void *page_to_virt(struct page *page) {
	extern volatile struct limine_hhdm_request hhdm_request;
	u64 hhdm_offset = hhdm_request.response->offset;
	return (void *)(hhdm_offset + page_to_phys(page));
}

/**
 * totalram_pages - Returns the total amount of physical memory managed
 */
u64 totalram_pages(void);

/**
 * freeram_bytes - Returns the current amount of free physical memory
 */
u64 freeram_pages(void);

/**
 * usedram_bytes - Returns the current amount of used physical memory
 */
u64 usedram_pages(void);

#endif // __NUCLEUS_MM_PMM_H