// SPDX-License-Identifier: Apache-2.0

#ifndef ARCH_IO_H
#define ARCH_IO_H

#include <seren/types.h>

/**
 * outb - Write a byte to an I/O port.
 * @port: The I/O port.
 * @value: The byte to write.
 */
static inline void outb(u16 port, u8 value);

/**
 * inb - Read a byte from an I/O port.
 * @port: The I/O port.
 *
 * Returns the byte read.
 */
static u8 inb(u16 port);

/**
 * io_wait - A small delay for older hardware.
 *
 * Some old devices need a short pause after an I/O operation.
 */
static void io_wait(void);

#if defined(ARCH_X86_64)
#include <port_io.h>
#else
#error "Unsupported architecture for I/O operations"
#endif

#endif // ARCH