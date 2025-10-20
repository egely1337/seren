/* SPDX-License-Identifier: Apache-2.0 */
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

/* TODO: Add KASLR support */

#include <asm-generic/linker.lds.h>

OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

KERNEL_BASE = 0xffffffff80000000;
KERNEL_PHYS_LOAD_ADDRESS = 0x100000;

PAGE_SIZE = 4K;

PHDRS
{
	phdr_limine_requests	PT_LOAD FLAGS(4); /* R */
	phdr_text		PT_LOAD FLAGS(5); /* R+X */
	phdr_rodata		PT_LOAD FLAGS(4); /* R */
	phdr_data		PT_LOAD FLAGS(6); /* R+W */

	phdr_init		PT_LOAD FLAGS(5); /* R+X */
}

SECTIONS
{
	. = KERNEL_BASE;

	/**
	 * Limine needs this to be at the very start of the file.
	 */
	.limine_requests ALIGN(PAGE_SIZE) : AT(KERNEL_PHYS_LOAD_ADDRESS) {
		KEEP(*(.limine_requests))
	} :phdr_limine_requests

	. = ALIGN(PAGE_SIZE);

	.text : {
		*(.text .text.*)
	} :phdr_text

	. = ALIGN(PAGE_SIZE);

	.rodata : {
		*(.rodata .rodata.*)
	} :phdr_rodata

	. = ALIGN(PAGE_SIZE);

	.data : {
		*(.data .data.*)
	} :phdr_data
	 
	.bss : {
		*(.bss .bss.*)
		*(COMMON)
	} :phdr_data

	. = ALIGN(PAGE_SIZE);

	__init_begin = .;

	.init : {
		. = ALIGN(4);
		*(.init.text)

		. = ALIGN(8);
		*(.init.rodata)
		__initcall_start = .;
		INIT_CALLS
		__initcall_end = .;

		. = ALIGN(4);
		*(.init.data)
	} :phdr_init

	. = ALIGN(PAGE_SIZE);
	__init_end = .;

	_kernel_end = .;

	/DISCARD/ : {
		*(.eh_frame*)
		*(.note .note.*)
		*(.comment)
	}
}