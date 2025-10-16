#define pr_fmt(fmt) "kheap: " fmt

#include <lib/string.h>
#include <nucleus/mm/kheap.h>
#include <nucleus/mm/pmm.h>
#include <nucleus/panic.h>
#include <nucleus/printk.h>
#include <nucleus/types.h>

#define HEAP_ALIGNMENT sizeof(void *)

static inline size_t align_up(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

typedef struct heap_block_header {
    size_t size;
    bool is_free;
    struct heap_block_header *next_free;
    struct heap_block_header *prev_phys;
    struct heap_block_header *next_phys;
} heap_block_header_t;

#define HEADER_SIZE align_up(sizeof(heap_block_header_t), HEAP_ALIGNMENT)

static heap_block_header_t *g_free_list_head = NULL;

static uintptr_t g_heap_start_addr = 0;
static uintptr_t g_heap_end_addr = 0;
static size_t g_heap_total_size = 0;

int kheap_init(void *initial_pool_start, size_t initial_pool_size) {
    pr_info("initializing kernel heap.\n");

    if (!initial_pool_start ||
        initial_pool_size < (HEADER_SIZE + HEAP_ALIGNMENT)) {
        pr_err("initial pool is too small or NULL.");
        return -1;
    }

    g_heap_start_addr = align_up((uintptr_t)initial_pool_start, PAGE_SIZE);
    uintptr_t actual_pool_start_after_align = (uintptr_t)initial_pool_start;
    size_t alignment_diff = g_heap_start_addr - actual_pool_start_after_align;

    if (alignment_diff > initial_pool_size ||
        initial_pool_size - alignment_diff < (HEADER_SIZE + HEAP_ALIGNMENT)) {
        pr_err("initial pool is too small after aligning start address.\n");
        g_heap_start_addr = 0;
        return -1;
    }

    g_heap_total_size = initial_pool_size - alignment_diff;
    g_heap_end_addr = g_heap_start_addr + g_heap_total_size;

    g_free_list_head = (heap_block_header_t *)g_heap_start_addr;
    g_free_list_head->size = g_heap_total_size - HEADER_SIZE;
    g_free_list_head->is_free = true;
    g_free_list_head->next_free = NULL;
    g_free_list_head->prev_phys = NULL;
    g_free_list_head->next_phys = NULL;

    pr_info("heap initialized at 0x%p, size: %lu KiB\n",
            (void *)g_heap_start_addr, g_heap_total_size / 1024);
    pr_debug("first free block created, size: %lu bytes.\n",
             g_free_list_head->size);

    return 0;
}

void *kmalloc(size_t size) {
    // TODO: Add a spinlock here for SMP safety

    if (size == 0) {
        pr_warn("kmalloc(0) called.\n");
        return NULL;
    }

    if (!g_free_list_head) {
        pr_warn("kmalloc called before heap is initialized or heap is full.\n");
        return NULL;
    }

    size_t actual_data_size = align_up(size, HEAP_ALIGNMENT);

    // Find first fit
    heap_block_header_t *current = g_free_list_head;
    heap_block_header_t *prev_free = NULL;

    while (current) {
        if (current->is_free && current->size >= actual_data_size) {
            // Found a suitable block

            // Check if we can split the block
            if (current->size >=
                actual_data_size + HEADER_SIZE + HEAP_ALIGNMENT) {
                heap_block_header_t *remaining_free_header =
                    (heap_block_header_t *)((uintptr_t)current + HEADER_SIZE +
                                            actual_data_size);

                remaining_free_header->size =
                    current->size - actual_data_size - HEADER_SIZE;
                remaining_free_header->is_free = true;
                remaining_free_header->next_free = current->next_free;

                remaining_free_header->prev_phys = current;
                remaining_free_header->next_phys = current->next_phys;
                if (current->next_phys) {
                    current->next_phys->prev_phys = remaining_free_header;
                }
                current->next_phys = remaining_free_header;

                current->size = actual_data_size;
                current->is_free = false;

                if (prev_free) {
                    prev_free->next_free = remaining_free_header;
                } else {
                    g_free_list_head = remaining_free_header;
                }
            } else {
                current->is_free = false;
                if (prev_free) {
                    prev_free->next_free = current->next_free;
                } else {
                    g_free_list_head = current->next_free;
                }
            }

            void *user_ptr = (void *)((uintptr_t)current + HEADER_SIZE);
            pr_debug("allocated %lu bytes at %p\n", actual_data_size, user_ptr);
            return user_ptr;
        }

        prev_free = current;
        current = current->next_free;
    }

    pr_warn("failed to find suitable block for size %lu (aligned %u).\n");

    // TODO: Implement heap expansion.
    return NULL;
}

void kfree(void *ptr) {
    if (!ptr) {
        return; // kfree(NULL)
    }

    // TODO: Add a spinlock here for SMP safety

    heap_block_header_t *header =
        (heap_block_header_t *)((uintptr_t)ptr - HEADER_SIZE);

    if (header->is_free) {
        panic("double free detected for pointer %p!\n", ptr);
        return;
    }

    header->is_free = true;
    pr_debug("freed block at %p, size %lu\n", ptr, header->size);

    if (header->next_phys && header->next_phys->is_free) {
        heap_block_header_t *curr_free = g_free_list_head;
        heap_block_header_t *prev_free = NULL;
        while (curr_free) {
            if (curr_free == header->next_phys) {
                if (prev_free)
                    prev_free->next_free = curr_free->next_free;
                else
                    g_free_list_head = curr_free->next_free;
                break;
            }
            prev_free = curr_free;
            curr_free = curr_free->next_free;
        }

        header->size += HEADER_SIZE + header->next_phys->size;
        header->next_phys = header->next_phys->next_phys;
        if (header->next_phys) {
            header->next_phys->prev_phys = header;
        }
    }

    header->next_free = g_free_list_head;
    g_free_list_head = header;
}

void *kcalloc(size_t num, size_t size) {
    size_t total_size = num * size;

    if (size != 0 && total_size / size != num) {
        pr_warn("integer overflow detected in kcalloc(num=%lu, size=%lu)", num,
                size);
        return NULL;
    }

    void *ptr = kmalloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}