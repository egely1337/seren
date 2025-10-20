# SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

.section .text

.equ GDT_NULL_SELECTOR,		0x00
.equ GDT_KERNEL_CODE_SELECTOR,	0x08
.equ GDT_KERNEL_DATA_SELECTOR,	0x10
.equ GDT_USER_CODE_SELECTOR,	0x18
.equ GDT_USER_DATA_SELECTOR,	0x20
.equ GDT_TSS_SELECTOR,		0x28


/**
 * gdt_flush - Load a new GDT and refresh segment registers.
 * %rdi: Pointer to a gdt_ptr structure.
 *
 * After changing the GDT the CPU doesn't automatically start using it.
 * We need to explicitly load the GDT pointer with `lgdt` and reload
 * all the segment registers to make the changes take effect.
 */
.global gdt_flush
gdt_flush:
	lgdt (%rdi)

	/**
	 * Now, reload all the data segment registers. We can't directly
	 * move an immediate value into them, so we have to go through `ax`.
	 */	
	movw $GDT_KERNEL_DATA_SELECTOR, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	
	/**
	 * Reloading the code segment register (%cs) is a bit trickier.
	 * We have to do a "far return" (`retfq`). We simulate this by
	 * pushing the new code selector and the address of the next
	 * instruction onto the stack.
	 */
	pushq $GDT_KERNEL_CODE_SELECTOR
	leaq .reload_cs(%rip), %rax
	pushq %rax
	retfq

.reload_cs:
	// We land here after the fat return above, with %cs reloaded.
	ret

/*
 * tss_flush - Load the Task Register (TR).
 * @di: The GDT selector for the TSS.
 */
.global tss_flush
tss_flush:
	movw %di, %ax
	ltrw %ax
	ret	
