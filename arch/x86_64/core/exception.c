#include <nucleus/panic.h>
#include <nucleus/printk.h>
#include <nucleus/types.h>

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    uint64_t interrupt_number;
    uint64_t error_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
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
void exception_handler(irq_context_t *frame) {
    const char *message = "Unknown Exception";

    if (frame->vector_number < (sizeof(exception_messages) / sizeof(char *)) &&
        exception_messages[frame->vector_number] != NULL) {
        message = exception_messages[frame->vector_number];
    }

    panic(message, frame);
}