#ifndef NUCLEUS_PANIC_H
#define NUCLEUS_PANIC_H

#include <nucleus/interrupt.h>

/**
 * @brief Halts the kernel.
 */
void panic(const char *message, struct pt_regs *context)
    __attribute__((noreturn));

#endif // NUCLEUS_PANIC_H