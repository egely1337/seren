// SPDX-License-Identifier: Apache-2.0
#ifndef _SEREN_MM_H
#define _SEREN_MM_H

#include <seren/types.h>

/**
 * kmalloc - Allocate a block of memory from the kernel heap.
 * @size: The number of bytes to allocate.
 *
 * Allocates at least @size bytes of physically contiguous memory.
 */
void *kmalloc(size_t size);

/**
 * kfree - Free a block of memory.
 * @ptr: The pointer to the memory block to free.
 */
void kfree(void *ptr);

/**
 * kcalloc - Allocate and zero-initialize an array in memory.
 * @num: The number of elements to allocate.
 * @size: The size of each element in bytes.
 */
void *kcalloc(size_t num, size_t size);

#endif // _SEREN_MM_H