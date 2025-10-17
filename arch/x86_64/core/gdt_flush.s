section .text

%define GDT_NULL_SELECTOR        0x00
%define GDT_KERNEL_CODE_SELECTOR 0x08
%define GDT_KERNEL_DATA_SELECTOR 0x10
%define GDT_USER_CODE_SELECTOR   0x18
%define GDT_USER_DATA_SELECTOR   0x20
%define GDT_TSS_SELECTOR         0x28

; gdt_flush - Load the Global Descriptor Table
; 
; This function loads the GDT using the LGDT instruction and
; reloads all segment registers to use the new GDT entries.
; 
; @rdi: Pointer to gdt_ptr structure
global gdt_flush
gdt_flush:
	lgdt [rdi]

	mov ax, GDT_KERNEL_DATA_SELECTOR
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	push GDT_KERNEL_CODE_SELECTOR
	lea rax, [rel .reload_cs]
	push rax
	retfq

.reload_cs:
	ret

; tss_flush - Load the Task State Segment
; 
; @rdi: TSS selector value
global tss_flush
tss_flush:
	mov ax, di
	ltr ax
	ret
