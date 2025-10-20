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

# gdt_flush - Load the GDT
#
# @rdi: Pointer to gdt_ptr structure
.global gdt_flush
gdt_flush:
	lgdt (%rdi)
	
	movw $GDT_KERNEL_DATA_SELECTOR, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	
	pushq $GDT_KERNEL_CODE_SELECTOR
	leaq .reload_cs(%rip), %rax
	pushq %rax
	retfq

.reload_cs:
	ret

# tss_flush - Load the TSS
#
# @di: holds the TSS selector
.global tss_flush
tss_flush:
	movw %di, %ax
	ltrw %ax
	ret	
