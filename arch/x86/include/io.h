// SPDX-License-Identifier: Apache-2.0

#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <nucleus/types.h>

/**
 * @brief Writes a byte to the specified I/O port.
 * (This is a declaration; implementation is arch-specific)
 */
static inline void outb(u16 port, u8 value);

/**
 * @brief Reads a byte from the specified I/O port.
 * (This is a declaration; implementation is arch-specific)
 */
static u8 inb(u16 port);

/**
 * @brief A short I/O delay.
 * (This is a declaration; implementation is arch-specific)
 */
static void io_wait(void);

#if defined(ARCH_X86_64)
#include <port_io.h>
#else
#error "Unsupported architecture for I/O operations"
#endif

#endif