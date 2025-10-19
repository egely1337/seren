// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <nucleus/panic.h>
#include <nucleus/printk.h>
#include <nucleus/stddef.h>
#include <nucleus/types.h>

typedef struct {
	u64 r15, r14, r13, r12, r11, r10, r9, r8;
	u64 rdi, rsi, rbp, rbx, rdx, rcx, rax;
	u64 interrupt_number;
	u64 error_code;

	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
} __attribute__((packed)) interrupt_frame_t;

const char *exception_messages[] = {"Divide by Zero Error",
				    "Debug",
				    "Non-Maskable Interrupt",
				    "Breakpoint",
				    "Overflow",
				    "Bound Range Exceeded",
				    "Invalid Opcode",
				    "Device Not Available",
				    "Double Fault",
				    "Coprocessor Segment Overrun",
				    "Invalid TSS",
				    "Segment Not Present",
				    "Stack-Segment Fault",
				    "General Protection Fault",
				    "Page Fault",
				    "Reserved (15)",
				    "x87 Floating-Point Exception",
				    "Alignment Check",
				    "Machine Check",
				    "SIMD Floating-Point Exception",
				    "Virtualization Exception",
				    "Control Protection Exception"};
void exception_handler(struct pt_regs *frame) {
	const char *message = "Unknown Exception";

	if (frame->vector < (sizeof(exception_messages) / sizeof(char *)) &&
	    exception_messages[frame->vector] != NULL) {
		message = exception_messages[frame->vector];
	}

	die(message, frame);
}