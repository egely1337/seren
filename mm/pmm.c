// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "pmm: " fmt

#include <limine.h>
#include <seren/config.h>
#include <seren/init.h>
#include <seren/mm/pmm.h>
#include <seren/panic.h>
#include <seren/printk.h>
#include <seren/types.h>

static unsigned long *bitmap;
static struct page *mem_map;

static unsigned long max_pfn;
static unsigned long nr_free;

extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_memmap_request memmap_request;

extern char _kernel_end[];

/**
 * These are the low-level helpers for manipulating the page bitmap.
 * `pfn >> 6` finds the u64 in the array and `pfn & 63` finds the bit within
 * that u64.
 */
static inline void __set_bit(u64 pfn) {
	if (pfn >= max_pfn)
		return;
	bitmap[pfn >> 6] |= (1UL << (pfn & 63));
}
static inline void __clear_bit(u64 pfn) {
	if (pfn >= max_pfn) /* Out of bounds is considered "used" */
		return;
	bitmap[pfn >> 6] &= ~(1UL << (pfn & 63));
}
static inline int __test_bit(u64 pfn) {
	if (pfn >= max_pfn)
		return 1;
	return (bitmap[pfn >> 6] & (1UL << (pfn & 63))) != 0;
}

/**
 * __find_free_pages - A irst-fit search for a contiguous block of pages.
 */
static s64 __find_free_pages(size_t count) {
	u64 consecutive = 0;
	u64 start_pfn = 0;

	if (!count || !bitmap)
		return -1;

	for (u64 pfn = 0; pfn < max_pfn; pfn++) {
		if (!__test_bit(pfn)) {
			if (consecutive == 0)
				start_pfn = pfn;
			consecutive++;
			if (consecutive == count)
				return start_pfn;
		} else {
			consecutive = 0;
		}
	}

	return -1;
}

static void __mark_pages_inuse(u64 start_pfn, size_t count) {
	for (size_t i = 0; i < count; i++) {
		__set_bit(start_pfn + i);
		nr_free--;
	}
}

static void __mark_pages_free(u64 start_pfn, size_t count) {
	for (size_t i = 0; i < count; i++) {
		if (__test_bit(start_pfn + i)) {
			__clear_bit(start_pfn + i);
			nr_free++;
		}
	}
}

/* Fills out our `mem_map` array so phys_to_page() works. */
static void __init_mem_map(u64 hhdm_offset, phys_addr_t mem_map_phys) {
	mem_map = (struct page *)(hhdm_offset + mem_map_phys);

	for (u64 pfn = 0; pfn < max_pfn; pfn++) {
		mem_map[pfn].pfn = pfn;
	}
}

/*
 * The PMM needs to allocate its own metadata (the bitmap and mem_map) and the
 * kernel. This function finds a chunk of usable memory large enough to hold
 * them.
 */
static phys_addr_t
__find_metadata_location(volatile struct limine_memmap_request *memmap_request,
			 phys_addr_t kernel_end, size_t required_size) {
	struct limine_memmap_response *memmap = memmap_request->response;

	for (u64 i = 0; i < memmap->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap->entries[i];

		if (entry->type != LIMINE_MEMMAP_USABLE)
			continue;

		u64 region_start = entry->base;
		u64 region_end = entry->base + entry->length;

		/* Make sure we're not overwriting the kernel. */
		if (region_start < kernel_end)
			region_start = kernel_end;

		region_start = (region_start + PAGE_SIZE - 1) & PAGE_MASK;

		if (region_start < region_end &&
		    (region_end - region_start) >= required_size)
			return region_start;
	}

	return 0;
}

/*
 * Figure out where the kernel's image ends in physical memory. We prefer to
 * use Limine's info, but fall back to the linker symbol if we have to.
 */
static phys_addr_t
__get_kernel_end(volatile struct limine_memmap_request *memmap_request) {
	struct limine_memmap_response *memmap = memmap_request->response;
	phys_addr_t kernel_start = KERNEL_PHYSICAL_LOAD_ADDR;
	phys_addr_t kernel_end = 0;

	for (u64 i = 0; i < memmap->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap->entries[i];

		if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
			if (entry->base <= kernel_start &&
			    (entry->base + entry->length) > kernel_start) {
				kernel_end = entry->base + entry->length;
				break;
			}
		}
	}

	if (!kernel_end) {
		uintptr_t kernel_virt_end = (uintptr_t)_kernel_end;
		kernel_end = KERNEL_PHYSICAL_LOAD_ADDR +
			     (kernel_virt_end - KERNEL_VIRTUAL_BASE);
		pr_warn("using linker symbols for kernel end\n");
	}

	return (kernel_end + PAGE_SIZE - 1) & PAGE_MASK;
}

/**
 * Reserve pages that shouldn't be allocated:
 * - Non-usable memory regions
 * - Kernel image
 * - Allocator metadata (bitmap + mem_map)
 */
static void
__reserve_system_pages(volatile struct limine_memmap_request *memmap_request,
		       phys_addr_t kernel_end, phys_addr_t metadata_phys,
		       size_t metadata_size) {
	struct limine_memmap_response *memmap = memmap_request->response;

	for (u64 pfn = 0; pfn < max_pfn; pfn++)
		__set_bit(pfn);

	for (u64 i = 0; i < memmap->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap->entries[i];

		if (entry->type == LIMINE_MEMMAP_USABLE ||
		    entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
			u64 start_pfn = entry->base >> PAGE_SHIFT;
			u64 end_pfn =
			    (entry->base + entry->length) >> PAGE_SHIFT;

			for (u64 pfn = start_pfn;
			     pfn < end_pfn && pfn < max_pfn; pfn++)
				__clear_bit(pfn);
		}
	}

	nr_free = 0;
	for (u64 pfn = 0; pfn < max_pfn; pfn++) {
		if (!__test_bit(pfn))
			nr_free++;
	}

	u64 kernel_start_pfn = KERNEL_PHYSICAL_LOAD_ADDR >> PAGE_SHIFT;
	u64 kernel_end_pfn = kernel_end >> PAGE_SHIFT;

	for (u64 pfn = kernel_start_pfn; pfn < kernel_end_pfn; pfn++) {
		if (!__test_bit(pfn)) {
			__set_bit(pfn);
			nr_free--;
		}
	}

	pr_debug("reserved %lu pages for kernel\n",
		 kernel_end_pfn - kernel_start_pfn);

	u64 metadata_start_pfn = metadata_phys >> PAGE_SHIFT;
	u64 metadata_pages = (metadata_size + PAGE_SIZE - 1) >> PAGE_SHIFT;

	for (u64 i = 0; i < metadata_pages; i++) {
		u64 pfn = metadata_start_pfn + i;
		if (!__test_bit(pfn)) {
			__set_bit(pfn);
			nr_free--;
		}
	}

	pr_debug("reserved %lu pages for allocator metadata\n", metadata_pages);
}

void mem_init(volatile struct limine_memmap_request *memmap_request) {
	struct limine_memmap_response *memmap;
	u64 hhdm_offset;
	phys_addr_t kernel_end;
	phys_addr_t metadata_phys;
	size_t bitmap_size, mem_map_size, metadata_size;

	if (!memmap_request || !memmap_request->response)
		panic("invalid memmap request", NULL);

	if (!hhdm_request.response)
		panic("HHDM not available", NULL);

	memmap = memmap_request->response;
	hhdm_offset = hhdm_request.response->offset;

	pr_debug("HHDM offset: 0x%lx\n", hhdm_offset);
	pr_debug("memory map entries: %lu\n", memmap->entry_count);

	max_pfn = 0;
	for (u64 i = 0; i < memmap->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap->entries[i];

		if (entry->type == LIMINE_MEMMAP_USABLE ||
		    entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
			u64 end_addr = entry->base + entry->length;
			u64 end_pfn = (end_addr + PAGE_SIZE - 1) >> PAGE_SHIFT;

			if (end_pfn > max_pfn)
				max_pfn = end_pfn;
		}
	}

	if (!max_pfn)
		panic("no usable memory found", NULL);

	pr_debug("managing %lu pages (0x%lx bytes)\n", max_pfn,
		 max_pfn << PAGE_SHIFT);

	bitmap_size = (max_pfn + 63) / 64 * sizeof(unsigned long);
	mem_map_size = max_pfn * sizeof(struct page);
	metadata_size = bitmap_size + mem_map_size;

	pr_debug("bitmap size: %lu bytes\n", bitmap_size);
	pr_debug("mem_map size: %lu bytes\n", mem_map_size);
	pr_debug("total metadata: %lu KiB\n", metadata_size / 1024);

	kernel_end = __get_kernel_end(memmap_request);
	pr_debug("kernel ends at: 0x%lx\n", kernel_end);

	metadata_phys =
	    __find_metadata_location(memmap_request, kernel_end, metadata_size);
	if (!metadata_phys)
		panic("cannot allocate space for PMM metadata", NULL);

	pr_info("metadata at physical 0x%lx\n", metadata_phys);

	bitmap = (unsigned long *)(hhdm_offset + metadata_phys);

	for (size_t i = 0; i < bitmap_size / sizeof(unsigned long); i++)
		bitmap[i] = 0;

	__reserve_system_pages(memmap_request, kernel_end, metadata_phys,
			       metadata_size);

	__init_mem_map(hhdm_offset, metadata_phys + bitmap_size);

	pr_info("initialization complete\n");
	pr_info("total: %lu MiB, free: %lu MiB, used: %lu MiB\n",
		(max_pfn << PAGE_SHIFT) >> 20, (nr_free << PAGE_SHIFT) >> 20,
		((max_pfn - nr_free) << PAGE_SHIFT) >> 20);
}

struct page *alloc_pages(u32 order) {
	size_t count = 1UL << order;
	s64 start_pfn;

	if (unlikely(!bitmap)) {
		pr_warn("allocator not initialized\n");
		return NULL;
	}

	if (unlikely(nr_free < count)) {
		pr_warn("out of memory (need %lu pages, have %lu)\n", count,
			nr_free);
		return NULL;
	}

	start_pfn = __find_free_pages(count);
	if (unlikely(start_pfn < 0)) {
		pr_warn("cannot find %lu contiguous pages\n", count);
		return NULL;
	}

	__mark_pages_inuse(start_pfn, count);

	pr_debug("allocated %lu pages at PFN 0x%lx\n", count, start_pfn);

	return &mem_map[start_pfn];
}

void free_pages(struct page *page, u32 order) {
	size_t count = 1UL << order;
	u64 start_pfn;

	if (unlikely(!bitmap || !page))
		return;

	start_pfn = page->pfn;

	if (unlikely(start_pfn >= max_pfn)) {
		pr_warn("invalid page frame number 0x%lx\n", start_pfn);
		return;
	}

	for (size_t i = 0; i < count; i++) {
		if (unlikely(!__test_bit(start_pfn + i))) {
			pr_warn("double free detected at PFN 0x%lx\n",
				start_pfn + i);
			return;
		}
	}

	__mark_pages_free(start_pfn, count);

	pr_debug("freed %lu pages at PFN 0x%lx\n", count, start_pfn);
}

phys_addr_t page_to_phys(struct page *page) {
	if (!page)
		return 0;
	return page->pfn << PAGE_SHIFT;
}

struct page *phys_to_page(phys_addr_t phys) {
	u64 pfn = phys >> PAGE_SHIFT;

	if (pfn >= max_pfn)
		return NULL;

	return &mem_map[pfn];
}

u64 totalram_pages(void) { return max_pfn << PAGE_SHIFT; }

u64 freeram_pages(void) { return nr_free << PAGE_SHIFT; }

u64 usedram_pages(void) { return (max_pfn - nr_free) << PAGE_SHIFT; }

static int __init pmm_setup(void) {
	mem_init(&memmap_request);

	return 0;
}

core_initcall(pmm_setup);