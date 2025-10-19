# SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

.intel_syntax noprefix

#define ISR_NO_ERR_CODE(vector_num)	\
	.global isr##vector_num;	\
	isr##vector_num:		\
	cli;				\
	push 0;				\
	jmp common_isr_stub

#define ISR_ERR_CODE(vector_num)	\
	.global isr##vector_num;	\
	isr##vector_num:		\
	cli;				\
	push vector_num;		\
	jmp common_isr_stub

#define IRQ_HANDLER_STUB(name, irq_num)	\
	.global irq_stub_##name;	\
	irq_stub_##name:		\
	cli;				\
	push 0;				\
	push (32 + irq_num);		\
	jmp common_irq_dispatcher_stub

.section .text

common_isr_stub:
	push rax; push rcx; push rdx; push rbx
	push rbp; push rsi; push rdi
	push r8;  push r9;  push r10; push r11
	push r12; push r13; push r14; push r15
	
	mov rdi, rsp
	call exception_handler
	
	pop r15; pop r14; pop r13; pop r12
	pop r11; pop r10; pop r9;  pop r8
	pop rdi; pop rsi; pop rbp; pop rbx
	pop rdx; pop rcx; pop rax
	
	add rsp, 16
	sti
	iretq


common_irq_dispatcher_stub:
	push rax; push rcx; push rdx; push rbx
	push rbp; push rsi; push rdi
	push r8;  push r9;  push r10; push r11
	push r12; push r13; push r14; push r15
	
	mov rdi, rsp
	call do_irq
	
	pop r15; pop r14; pop r13; pop r12
	pop r11; pop r10; pop r9;  pop r8
	pop rdi; pop rsi; pop rbp; pop rbx
	pop rdx; pop rcx; pop rax
	
	add rsp, 16
	sti
	iretq

.global irq_stub_0
irq_stub_0:
	cli
	push 0
	push 32
	
	push rax; push rcx; push rdx; push rbx
	push rbp; push rsi; push rdi
	push r8;  push r9;  push r10; push r11
	push r12; push r13; push r14; push r15
	
	mov rdi, rsp
	call do_irq
	
	mov rdi, rsp
	call schedule
	mov rsp, rax # CONTEXT SWITCH
	
	pop r15; pop r14; pop r13; pop r12
	pop r11; pop r10; pop r9;  pop r8
	pop rdi; pop rsi; pop rbp; pop rbx
	pop rdx; pop rcx; pop rax
	
	add rsp, 16
	sti
	iretq

ISR_NO_ERR_CODE(0)
ISR_NO_ERR_CODE(1)
ISR_NO_ERR_CODE(2)
ISR_NO_ERR_CODE(3)
ISR_NO_ERR_CODE(4)
ISR_NO_ERR_CODE(5)
ISR_NO_ERR_CODE(6)
ISR_NO_ERR_CODE(7)
ISR_ERR_CODE(8)
ISR_NO_ERR_CODE(9)
ISR_ERR_CODE(10)
ISR_ERR_CODE(11)
ISR_ERR_CODE(12)
ISR_ERR_CODE(13)
ISR_ERR_CODE(14)
# ISR 15 is reserved by Intel.
ISR_NO_ERR_CODE(16)
ISR_ERR_CODE(17)
ISR_NO_ERR_CODE(18)
ISR_NO_ERR_CODE(19)
ISR_NO_ERR_CODE(20)

IRQ_HANDLER_STUB(1, 1)   # Keyboard
IRQ_HANDLER_STUB(2, 2)   # Cascade for Slave PIC
IRQ_HANDLER_STUB(3, 3)   # COM2
IRQ_HANDLER_STUB(4, 4)   # COM1
IRQ_HANDLER_STUB(5, 5)   # LPT2
IRQ_HANDLER_STUB(6, 6)   # Floppy
IRQ_HANDLER_STUB(7, 7)   # LPT1 / Spurious
IRQ_HANDLER_STUB(8, 8)   # RTC
IRQ_HANDLER_STUB(9, 9)   # Free / ACPI
IRQ_HANDLER_STUB(10, 10)  # Free
IRQ_HANDLER_STUB(11, 11)  # Free
IRQ_HANDLER_STUB(12, 12)  # PS/2 Mouse
IRQ_HANDLER_STUB(13, 13)  # FPU
IRQ_HANDLER_STUB(14, 14)  # Primary ATA
IRQ_HANDLER_STUB(15, 15)  # Secondary ATA

.global idt_load
idt_load:
	lidt [rdi]
	ret