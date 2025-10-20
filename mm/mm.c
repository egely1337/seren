#define pr_fmt(fmt) "mm: " fmt

#include <lib/string.h>
#include <seren/init.h>
#include <seren/mm.h>
#include <seren/mm/pmm.h>
#include <seren/mm/slab.h>
#include <seren/panic.h>
#include <seren/printk.h>

void *kmalloc(size_t size) {
	if (unlikely(size == 0)) {
		return NULL;
	}

	if (size > (1UL << KMALLOC_MAX_SHIFT)) {
		pr_warn(
		    "kmalloc: large allocation (%zu bytes) not yet supported\n",
		    size);
		return NULL;
	}

	struct kmem_cache *cache = slab_get_kmalloc_cache(size);
	if (unlikely(!cache)) {
		return NULL;
	}

	return kmem_cache_alloc(cache);
}

void kfree(void *ptr) {
	struct slab *sl;
	struct page *pg;

	if (unlikely(!ptr)) {
		return;
	}

	pg = virt_to_page(ptr);
	sl = (struct slab *)page_to_virt(pg);

	if (sl->magic != SLAB_PAGE_MAGIC || !sl->cache) {
		panic("kfree: invalid pointer %p or slab metadata corruption",
		      ptr);
	}

	kmem_cache_free(sl->cache, ptr);
}

void *kcalloc(size_t num, size_t size) {
	void *p;
	size_t total_size;

	if (unlikely(num == 0 || size == 0)) {
		return NULL;
	}

	if (unlikely(__builtin_mul_overflow(num, size, &total_size))) {
		return NULL;
	}

	p = kmalloc(total_size);
	if (p) {
		memset(p, 0, total_size);
	}
	return p;
}

static int __init mm_init(void) {
	slab_init_kmalloc_caches();
	return 0;
}

postcore_initcall(mm_init); // Slab depends on PMM. So it comes after.