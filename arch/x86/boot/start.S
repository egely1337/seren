// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

.section .bss
	.align 16

stack_bottom:
	.skip 16384
stack_top:

.section .text
	.global _start

_start:
	movq $stack_top, %rsp

	/**
	 * And we're off! We jump into the high-level C part of the kernel.
	 */
	call kmain

.halt_loop:
	cli
	hlt
	jmp .halt_loop
