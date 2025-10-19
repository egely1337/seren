// SPDX-License-Identifier: Apache-2.0

#ifndef _ASM_X86_64_PROCESSOR_H
#define _ASM_X86_64_PROCESSOR_H

/**
 * cpu_relax - Hint to the CPU that we are in a spin-wait loop.
 */
static inline void cpu_relax(void) { __asm__ volatile("pause" ::: "memory"); }

#endif // _ASM_X86_64_PROCESSOR_H