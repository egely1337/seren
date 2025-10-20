// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_MM_SLAB_H
#define _SEREN_MM_SLAB_H

#include <seren/list.h>
#include <seren/stddef.h>
#include <seren/types.h>

#define KMALLOC_MIN_SHIFT  3
#define KMALLOC_MAX_SHIFT  12
#define KMALLOC_NUM_CACHES (KMALLOC_MAX_SHIFT - KMALLOC_MIN_SHIFT)

#define KMALLOC_MAX_SIZE (128 * 1024)

#define SLAB_PAGE_MAGIC 0x51ab51ab

/**
 * Forward declarations
 */
struct kmem_cache;
struct slab;

/**
 * struct kmem_cache - A cache for objects of a specific size.
 * @name: Human-readable name for debugging (e.g., "task_structs").
 * @size: The size of each object in this cache.
 * @align: The required alignment for each object.
 * @ctor: Optional constructor to run on new objects.
 * @dtor: Optional destructor to run when objects are freed.
 * @partial: A list of slabs that are partially full.
 * @full: A list of slabs that have no free objects.
 * @free: A list of slabs that are completely empty.
 * @nr_free_slabs: A count of slabs on the `free` list, for quick reclamation.
 */
struct kmem_cache {
	const char *name;
	size_t size;
	size_t align;
	void (*ctor)(void *);
	void (*dtor)(void *);
	struct list_head partial;
	struct list_head full;
	struct list_head free;
	unsigned int nr_free_slabs;
};

/**
 * struct slab - A container for one or more objects.
 * @list: Links this slab into one of the `kmem_cache` lists (partial, full,
 * free).
 * @cache: A back-pointer to the cache this slab belongs to.
 * @inuse: The number of objects currently allocated from this slab.
 * @total: The total number of objects this slab can hold.
 * @free_list: A simple, singly-linked list of the free objects within this
 * slab.
 * @page: A pointer to the `struct page` that this slab lives in.
 * @magic: The magic number for validation (SLAB_PAGE_MAGIC).
 */
struct slab {
	struct list_head list;
	struct kmem_cache *cache;
	u16 inuse;
	u16 total;
	void *free_list;
	struct page *page;
	u32 magic;
};

struct page_alloc_hdr {
	u32 magic;
	u32 order;
	struct page *first_page;
};

/**
 * kmem_cache_create - Create a new slab cache for objects of a specific size
 * @name: A human-readable name for the cache
 * @size: The size of each object to be stored in the cache
 * @align: The required alignment for the objects
 * @ctor: A constructor function to be called on new objects.
 * @dtor: A destructor function to be called on objects before they are freed.
 *
 * Returns a pointer to the newly created `struct kmem_cache` on success,
 * or NULL on failure.
 */
struct kmem_cache *kmem_cache_create(const char *name, size_t size,
				     size_t align, void (*ctor)(void *),
				     void (*dtor)(void *));

/**
 * kmem_cache_destroy - Destroy a slab cache.
 * @cache: The cache to destroy.
 *
 * Releases all memory associated with a slab cache back to the page allocator.
 * Attempting to destroy a cache with objects still in use will result in a
 * KERNEL PANIC.
 */
void kmem_cache_destroy(struct kmem_cache *cache);

/**
 * kmem_cache_alloc - Allocate an object from a slab cache.
 * @cache: The cache from which to allocate the object.
 */
void *kmem_cache_alloc(struct kmem_cache *cache);

/**
 * kmem_cache_free - Free an object and return it to its slab cache.
 * @cache: The cache to which the object belongs.
 * @objp: The object to free.
 */
void kmem_cache_free(struct kmem_cache *cache, void *objp);

/**
 * slab_get_kmalloc_cache - Find a general-purpose cache for a given size.
 */
struct kmem_cache *slab_get_kmalloc_cache(size_t size);

/**
 * slab_init_kmalloc_caches - Initialize the general-purpose kernel heap.
 */
int slab_init_kmalloc_caches();

#endif // _SEREN_MM_SLAB_H
