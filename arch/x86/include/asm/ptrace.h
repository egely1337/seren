// SPDX-License-Identifier: Apache-2.0

#ifndef _ASM_X86_64_PTRACE_H
#define _ASM_X86_64_PTRACE_H

#include <seren/types.h>

/**
 * struct pt_regs - CPU registers saved on kernel entry
 *
 * This structure defines the layout of the CPU registers as they are saved
 * on the stack when an exception/interrupt occurs. Our common interrupt
 * handler pushes these registers in a specific order so that a C function can
 * access the full context of the interrupted task.
 *
 * It's also used to set up the initial state for a new task before it runs
 * for the first time.
 */
struct pt_regs {
	/**
	 * Callee-saved registers.
	 */
	u64 r15, r14, r13, r12, rbp, rbx;

	/**
	 * Caller-saved registers.
	 */
	u64 r11, r10, r9, r8, rax, rcx, rdx, rsi, rdi;

	/**
	 * Pushed by the assembly stub.
	 */
	u64 vector;

	/**
	 * Pushed by the CPU for certain exceptions.
	 * Or a dummy value of 0 pushed by our assembly stub.
	 */
	u64 error_code;

	/**
	 * Pushed by the CPU on interrupt or exception entry.
	 * This is the "iret frame"
	 */
	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
};

#endif // _ASM_X86_64_PTRACE_H