#include <stdint.h>
#include <nucleus/printk.h>

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

const char *exception_messages[] = {
    "Divide by Zero Error"
};

void exception_handler(interrupt_frame_t *frame) {
    printk("\n!! KERNEL EXCEPTION !!\n");

    if (frame->interrupt_number < (sizeof(exception_messages) / sizeof(char*)) && exception_messages[frame->interrupt_number] != NULL) {
        printk("Exception (0x%x): %s\n", frame->interrupt_number, (char*)exception_messages[frame->interrupt_number]);
    } else {
        printk("Unknown Exception (%x)", frame->interrupt_number);
    }

    printk(KERN_CRIT "Registers:\n");
    printk(KERN_CRIT "  RIP: %p  RSP: %p  RFLAGS: %p\n",
           (void*)frame->rip, (void*)frame->rsp, (void*)frame->rflags);
    printk(KERN_CRIT "  RAX: %p  RBX: %p  RCX: %p  RDX: %p\n",
           (void*)frame->rax, (void*)frame->rbx, (void*)frame->rcx, (void*)frame->rdx);
    printk(KERN_CRIT "  RSI: %p  RDI: %p  RBP: %p\n",
           (void*)frame->rsi, (void*)frame->rdi, (void*)frame->rbp);
    printk(KERN_CRIT "  R8:  %p  R9:  %p  R10: %p  R11: %p\n",
           (void*)frame->r8,  (void*)frame->r9,  (void*)frame->r10, (void*)frame->r11);
    printk(KERN_CRIT "  R12: %p  R13: %p  R14: %p  R15: %p\n",
           (void*)frame->r12, (void*)frame->r13, (void*)frame->r14, (void*)frame->r15);
    printk(KERN_CRIT "  CS:  %p  SS:  %p\n",
           (void*)frame->cs,  (void*)frame->ss);

    printk(KERN_EMERG "System halted.\n");

    while(1) {
        __asm__ volatile ("cli; hlt");
    }
}