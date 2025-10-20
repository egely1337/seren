// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "slab: " fmt

#include <lib/string.h>
#include <seren/init.h>
#include <seren/list.h>
#include <seren/mm.h>
#include <seren/mm/pmm.h>
#include <seren/mm/slab.h>
#include <seren/panic.h>
#include <seren/printk.h>
#include <seren/stddef.h>
#include <seren/types.h>

static inline size_t __align_up(size_t size, size_t align) {
	return (size + align - 1) & ~(align - 1);
}

/**
 * We bould our free lists by storing a pointer to the next free object
 * inside the storage of the current free object.
 */
static inline void *__obj_set_next(void *obj, void *next) {
	*(void **)obj = next;
	return obj;
}

static inline void *__obj_get_next(void *obj) { return *(void **)obj; }

static struct kmem_cache kmalloc_caches[KMALLOC_NUM_CACHES];
static int kmalloc_caches_inited;

static inline struct slab *__list_to_slab(struct list_head *head) {
	return (struct slab *)((char *)head - offsetof(struct slab, list));
}
static inline void __slab_list_add(struct slab *sl, struct list_head *head) {
	list_add(&sl->list, head);
}
static inline void __slab_list_del(struct slab *sl) { list_del(&sl->list); }
static inline struct slab *__slab_list_first(struct list_head *head) {
	if (list_empty(head))
		return NULL;
	return __list_to_slab(head->next);
}

/**
 * __slab_create - Allocate a new page and format it as a slab.
 * @cache: The cache that this new slab will belong to.
 *
 * This is called when a cache runs out of free objects and needs more memory.
 * It gets a fresh page from the PMM, places a `struct slab` header at the
 * start, and then carves up the rest of the page into objects, adding them
 * all to an internal free list.
 */
static struct slab *__slab_create(struct kmem_cache *cache) {
	struct page *page = alloc_page();
	void *page_base;
	struct slab *sl;
	uintptr_t cursor;
	uintptr_t end;
	unsigned int count = 0;

	if (!page)
		return NULL;

	page_base = page_to_virt(page);
	sl = (struct slab *)page_base;
	sl->magic = SLAB_PAGE_MAGIC;
	sl->cache = cache;
	sl->page = page;
	sl->inuse = 0;
	sl->free_list = NULL;
	INIT_LIST_HEAD(&sl->list);

	/* place objects after slab header */
	cursor = __align_up((uintptr_t)page_base + sizeof(struct slab),
			    cache->align);
	end = (uintptr_t)page_base + PAGE_SIZE;

	while (cursor + cache->size <= end) {
		void *obj = (void *)cursor;
		sl->free_list = __obj_set_next(obj, sl->free_list);
		count++;
		cursor += cache->size;
	}

	sl->total = (u16)count;
	if (count == 0) {
		free_page(page);
		return NULL;
	}

	pr_debug("kmem_cache '%s': new slab %p (objsz=%lu, total=%u)\n",
		 cache->name, sl, (unsigned long)cache->size, sl->total);

	return sl;
}

static void __slab_release(struct slab *sl) {
	if (sl->magic != SLAB_PAGE_MAGIC)
		panic("release: invalid slab magic", NULL);
	pr_debug("kmem_cache '%s': release slab %p\n", sl->cache->name, sl);
	free_page(sl->page);
}

static void *__slab_alloc(struct kmem_cache *cache) {
	struct slab *sl;
	void *obj;

	/**
	 * The allocation strategy is simple:
	 * 1. Try to get an object from a partially full slab.
	 * 2. If none, try to get one from a completely free slab.
	 * 3. If none, create a brand new slab.
	 */
	if (!list_empty(&cache->partial)) {
		sl = __slab_list_first(&cache->partial);
	} else if (!list_empty(&cache->free)) {
		sl = __slab_list_first(&cache->free);
	} else {
		sl = NULL;
	}

	if (!sl) {
		sl = __slab_create(cache);
		if (!sl)
			return NULL;
		__slab_list_add(sl, &cache->free);
		cache->nr_free_slabs++;
	}

	obj = sl->free_list;
	sl->free_list = __obj_get_next(obj);
	sl->inuse++;

	pr_debug("kmem_cache '%s': alloc obj %p (slab %p inuse=%u/%u)\n",
		 cache->name, obj, sl, sl->inuse, sl->total);

	/**
	 * If it just became full, move it to the `full` list.
	 * If it was free and is now partially used, move it to the `partial`
	 * list.
	 */
	if (sl->inuse == sl->total) { /* partial -> full */
		__slab_list_del(sl);
		__slab_list_add(sl, &cache->full);
		if (cache->nr_free_slabs)
			cache->nr_free_slabs--;

	} else if (!list_empty(&cache->free) && /* free -> partial */
		   sl == __slab_list_first(&cache->free)) {
		__slab_list_del(sl);
		__slab_list_add(sl, &cache->partial);
		if (cache->nr_free_slabs)
			cache->nr_free_slabs--;
	}

	if (cache->ctor)
		cache->ctor(obj);

	return obj;
}

static void __slab_try_reclaim(struct kmem_cache *cache) {
	while (cache->nr_free_slabs > 1 && !list_empty(&cache->free)) {
		struct slab *victim = __slab_list_first(&cache->free);
		pr_debug("kmem_cache '%s': reclaim slab %p (free_slabs=%u)\n",
			 cache->name, victim, cache->nr_free_slabs);
		__slab_list_del(victim);
		__slab_release(victim);
		cache->nr_free_slabs--;
	}
}

static void __slab_free(struct kmem_cache *cache, void *obj) {
	struct page *pg = virt_to_page(obj);
	void *page_base = page_to_virt(pg);
	struct slab *sl = (struct slab *)page_base;

	if (sl->magic != SLAB_PAGE_MAGIC || sl->cache != cache)
		panic("invalid slab free %p", obj);

	if (cache->dtor)
		cache->dtor(obj);

	__obj_set_next(obj, sl->free_list);
	sl->free_list = obj;

	if (sl->inuse == sl->total) {
		__slab_list_del(sl);
		__slab_list_add(sl, &cache->partial);
	}

	sl->inuse--;

	pr_debug("kmem_cache '%s': free obj %p (slab %p inuse=%u/%u)\n",
		 cache->name, obj, sl, sl->inuse, sl->total);

	if (sl->inuse == 0) {
		// move to free
		__slab_list_del(sl);
		__slab_list_add(sl, &cache->free);
		cache->nr_free_slabs++;
		__slab_try_reclaim(cache);
	}
}

struct kmem_cache *kmem_cache_create(const char *name, size_t size,
				     size_t align, void (*ctor)(void *),
				     void (*dtor)(void *)) {
	struct kmem_cache *c =
	    (struct kmem_cache *)kmalloc(sizeof(struct kmem_cache));
	if (!c)
		return NULL;

	if (align == 0)
		align = sizeof(void *);

	c->name = name;
	c->size = __align_up(size, align);
	c->align = align;
	c->ctor = ctor;
	c->dtor = dtor;
	INIT_LIST_HEAD(&c->partial);
	INIT_LIST_HEAD(&c->full);
	INIT_LIST_HEAD(&c->free);
	c->nr_free_slabs = 0;

	return c;
}

void kmem_cache_destroy(struct kmem_cache *cache) {
	struct slab *sl;

	if (!list_empty(&cache->partial) || !list_empty(&cache->full)) {
		panic("kmem_cache_destroy: attempting to destroy a cache with "
		      "active objects");
	}

	while (!list_empty(&cache->free)) {
		sl = __slab_list_first(&cache->free);
		__slab_list_del(sl);
		__slab_release(sl);
	}

	kfree(cache);
}

void *kmem_cache_alloc(struct kmem_cache *cache) { return __slab_alloc(cache); }

void kmem_cache_free(struct kmem_cache *cache, void *objp) {
	if (!objp)
		return;
	__slab_free(cache, objp);
}

static void __kmalloc_caches_init(void) {
	if (kmalloc_caches_inited)
		return;

	for (unsigned int i = 0; i < KMALLOC_NUM_CACHES; i++) {
		struct kmem_cache *c = &kmalloc_caches[i];
		size_t sz = (size_t)1 << (KMALLOC_MIN_SHIFT + i);
		c->name = "kmalloc";
		c->size = __align_up(sz, sizeof(void *));
		c->align = sizeof(void *);
		c->ctor = NULL;
		c->dtor = NULL;
		INIT_LIST_HEAD(&c->partial);
		INIT_LIST_HEAD(&c->full);
		INIT_LIST_HEAD(&c->free);
		c->nr_free_slabs = 0;
	}

	kmalloc_caches_inited = 1;
}

static inline int __size_to_index(size_t size) {
	int idx = 0;
	size_t s = (size_t)1 << KMALLOC_MIN_SHIFT;
	while (s < size && (KMALLOC_MIN_SHIFT + idx) < KMALLOC_MAX_SHIFT) {
		s <<= 1;
		idx++;
	}
	return idx;
}

int slab_init_kmalloc_caches(void) {
	__kmalloc_caches_init();
	pr_info("initialized kmalloc caches (%u classes)\n",
		KMALLOC_NUM_CACHES);
	return 0;
}

struct kmem_cache *slab_get_kmalloc_cache(size_t size) {
	if (unlikely(size > (1UL << KMALLOC_MAX_SHIFT))) {
		return NULL;
	}
	int idx = __size_to_index(size);
	if (idx >= (int)KMALLOC_NUM_CACHES) {
		return NULL;
	}
	return &kmalloc_caches[idx];
}
