#ifndef NUCLEUS_PANIC_H
#define NUCLEUS_PANIC_H

#include <nucleus/interrupt.h>

/**
 * @brief Halts the kernel.
 */
void panic(const char *message, irq_context_t *context)
    __attribute__((noreturn));

#endif // NUCLEUS_PANIC_H