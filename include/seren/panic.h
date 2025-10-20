// SPDX-License-Identifier: Apache-2.0
#ifndef _SEREN_PANIC_H
#define _SEREN_PANIC_H

#include <seren/interrupt.h>

/**
 * panic - Halts the kernel.
 * @fmt: A printf-style format string.
 * @...: Arguments for the format string.
 */
void panic(const char *fmt, ...) __attribute__((noreturn));

/**
 * die - Halts the kernel due to a fatal exception.
 * @msg:    A string describing the trap/exception.
 * @regs:   The register context at the time of the fault.
 *
 * This function is called by trap handlers for unrecoverable faults.
 */
void die(const char *msg, struct pt_regs *regs);

#endif // _SEREN_PANIC_H