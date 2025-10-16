#ifndef _NUCLEUS_MM_SLAB_H
#define _NUCLEUS_MM_SLAB_H

#include <nucleus/stddef.h>
#include <nucleus/types.h>

struct kmem_cache;

/**
 * kmem_cache_create - Create a new slab cache for objects of a specific size
 * @name:   A human-readable name for the cache
 * @size:   The size of each object to be stored in the cache
 * @align:  The required alignment for the objects
 * @ctor:   A constructor function to be called on new objects.
 * @dtor:   A destructor function to be called on objects before they are freed.
 *
 * Returns a pointer to the newly created `struct kmem_cache` on success,
 * or NULL on failure.
 */
struct kmem_cache *kmem_cache_create(const char *name, size_t size,
                                     size_t align, void (*ctor)(void *),
                                     void (*dtor)(void *));

/**
 * kmem_cache_destroy - Destroy a slab cache.
 * @cache:  The cache to destroy.
 *
 * Releases all memory associated with a slab cache back to the page allocator.
 * Attempting to destroy a cache with objects still in use will result in a
 * KERNEL PANIC.
 */
void kmem_cache_destroy(struct kmem_cache *cache);

/**
 * kmem_cache_alloc - Allocate an object from a slab cache.
 * @cache:  The cache from which to allocate the object.
 */
void *kmem_cache_alloc(struct kmem_cache *cache);

/**
 * kmem_cache_free - Free an object and return it to its slab cache.
 * @cache:  The cache to which the object belongs.
 * @objp:   The object to free.
 */
void kmem_cache_free(struct kmem_cache *cache, void *objp);

/**
 * kheap_init - Initialize the general-purpose kernel heap.
 * @initial_pool_start: (currently unused) The start address of a pre-allocated
 * memory pool.
 * @initial_pool_size: (currently unused) The size of the pre-allocated pool.
 */
int kheap_init(void *initial_pool_start, size_t initial_pool_size);

/**
 * kmalloc - Allocate a block of memory from the kernel heap.
 * @size:   The number of bytes to allocate.
 *
 * Allocates at least @size bytes of physically contiguous memory.
 */
void *kmalloc(size_t size);

/**
 * kfree - Free a block of memory.
 * @ptr:    The pointer to the memory block to free.
 */
void kfree(void *ptr);

/**
 * kcalloc - Allocate and zero-initialize an array in memory.
 * @num:    The number of elements to allocate.
 * @size:   The size of each element in bytes.
 */
void *kcalloc(size_t num, size_t size);

#endif // _NUCLEUS_MM_SLAB_H
