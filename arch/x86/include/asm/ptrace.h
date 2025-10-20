// SPDX-License-Identifier: Apache-2.0

#ifndef _ASM_X86_64_PTRACE_H
#define _ASM_X86_64_PTRACE_H

#include <seren/types.h>

struct pt_regs {
	u64 r15, r14, r13, r12, rbp, rbx;
	u64 r11, r10, r9, r8, rax, rcx, rdx, rsi, rdi;

	u64 vector;
	u64 error_code;

	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
};

#endif // _ASM_X86_64_PTRACE_H