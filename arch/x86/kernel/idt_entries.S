# SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <asm/irq_vectors.h>

/**
 * idt_entry - Macro to generate a unique assembly stup for each ISR.
 *
 * The CPU doesn't push the interrupt vector number or a consistent error
 * code onto stack for all interrupts (why, bro?). To create a uniform
 * C-callable interface (our `pt_regs` struct), we need these assembly stubs.
 *
 * Each stub does two things:
 * 1. Pushes a dummy error code (0) if the CPU doesn't provide one
 * 2. Pushes the interrupt vector number
 */
.macro idt_entry name, vector, has_err=0
.global \name
.align 8
\name:
    .if \has_err == 0
        pushq $0
    .endif

    pushq $\vector

    jmp common_interrupt_handler
.endm

/**
 * common_interrupt_handler - The shared assembly code for all interrupts.
 *
 * After an `idt_entry` stub prepares the stack, all interrupts land here.
 */
.global common_interrupt_handler
common_interrupt_handler:
	/**
	 * Disable interrupts to prevent nested interrupts.
	 * The CPU does this automatically for most interrupts, but it's
	 * good practice to be explicit.
	 */
	cli

	pushq %rdi; pushq %rsi; pushq %rdx; pushq %rcx;
	pushq %rax; pushq %r8;  pushq %r9;  pushq %r10;
	pushq %r11;

	pushq %rbx; pushq %rbp; pushq %r12; pushq %r13;
    	pushq %r14; pushq %r15;

	movq %rsp, %rdi
    	call handle_interrupt

	// If the rax is 0, no reschedule is needed
	testq %rax, %rax
	je .no_reschedule

	movq %rsp, %rdi
	call schedule
	movq %rax, %rsp // CONTEXT SWITCH HAPPENS HERE!!!!!

.no_reschedule:
	popq %r15; popq %r14; popq %r13; popq %r12;
    	popq %rbp; popq %rbx;
    	popq %r11; popq %r10; popq %r9;  popq %r8;
    	popq %rax; popq %rcx; popq %rdx; popq %rsi;
    	popq %rdi;

	addq $16, %rsp
	sti
	iretq

idt_entry isr0,  DIVIDE_ERROR_VECTOR
idt_entry isr1,  DEBUG_VECTOR
idt_entry isr2,  NMI_VECTOR
idt_entry isr3,  BREAKPOINT_VECTOR
idt_entry isr4,  OVERFLOW_VECTOR
idt_entry isr5,  BOUNDS_CHECK_VECTOR
idt_entry isr6,  INVALID_OPCODE_VECTOR
idt_entry isr7,  DEVICE_NOT_AVAILABLE_VECTOR
idt_entry isr8,  DOUBLE_FAULT_VECTOR, has_err=1
idt_entry isr9,  COPROCESSOR_SEGMENT_OVERRUN_VECTOR
idt_entry isr10, INVALID_TSS_VECTOR, has_err=1
idt_entry isr11, SEGMENT_NOT_PRESENT_VECTOR, has_err=1
idt_entry isr12, STACK_FAULT_VECTOR, has_err=1
idt_entry isr13, GENERAL_PROTECTION_FAULT_VECTOR, has_err=1
idt_entry isr14, PAGE_FAULT_VECTOR, has_err=1
// Vector 15 is reserved
idt_entry isr16, X87_FPU_VECTOR
idt_entry isr17, ALIGNMENT_CHECK_VECTOR, has_err=1
idt_entry isr18, MACHINE_CHECK_VECTOR
idt_entry isr19, SIMD_FP_VECTOR
idt_entry isr20, VIRTUALIZATION_VECTOR

idt_entry irq_stub_0,  PIC1_START_VECTOR
idt_entry irq_stub_1,  PIC1_START_VECTOR + 1
idt_entry irq_stub_2,  PIC1_START_VECTOR + 2
idt_entry irq_stub_3,  PIC1_START_VECTOR + 3
idt_entry irq_stub_4,  PIC1_START_VECTOR + 4
idt_entry irq_stub_5,  PIC1_START_VECTOR + 5
idt_entry irq_stub_6,  PIC1_START_VECTOR + 6
idt_entry irq_stub_7,  PIC1_START_VECTOR + 7
idt_entry irq_stub_8,  PIC2_START_VECTOR
idt_entry irq_stub_9,  PIC2_START_VECTOR + 1
idt_entry irq_stub_10, PIC2_START_VECTOR + 2
idt_entry irq_stub_11, PIC2_START_VECTOR + 3
idt_entry irq_stub_12, PIC2_START_VECTOR + 4
idt_entry irq_stub_13, PIC2_START_VECTOR + 5
idt_entry irq_stub_14, PIC2_START_VECTOR + 6
idt_entry irq_stub_15, PIC2_START_VECTOR + 7

.global idt_load
idt_load:
	lidt (%rdi)
	ret
