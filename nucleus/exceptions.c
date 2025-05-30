#include <stdint.h>
#include <nucleus/console.h>

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

static void print_hex(uint64_t n) {
    char buf[17];
    buf[16] = '\0';
    int i = 15;
    if (n == 0) {
        console_putchar('0');
        return;
    }
    while (n > 0 && i >= 0) {
        uint8_t digit = n % 16;
        if (digit < 10) {
            buf[i--] = '0' + digit;
        } else {
            buf[i--] = 'A' + (digit - 10);
        }
        n /= 16;
    }
    console_writestring(&buf[i + 1]);
}

const char *exception_messages[] = {
    "Divide by Zero Error"
};

void exception_handler(interrupt_frame_t *frame) {
    console_writestring("\n!! KERNEL EXCEPTION !!\n");

    if (frame->interrupt_number < (sizeof(exception_messages) / sizeof(char*)) && exception_messages[frame->interrupt_number] != NULL) {
        console_writestring("Exception (");
        print_hex(frame->interrupt_number);
        console_writestring("): ");
        console_writestring((char*)exception_messages[frame->interrupt_number]);
        console_putchar('\n');
    } else {
        console_writestring("Unknown Exception (");
        print_hex(frame->interrupt_number);
        console_writestring(")\n");
    }

    console_writestring("Registers:\n");
    console_writestring("  RIP: 0x"); print_hex(frame->rip);
    console_writestring("  RSP: 0x"); print_hex(frame->rsp);
    console_writestring("  RFLAGS: 0x"); print_hex(frame->rflags);
    console_putchar('\n');
    console_writestring("  RAX: 0x"); print_hex(frame->rax);
    console_writestring("  RBX: 0x"); print_hex(frame->rbx);
    console_writestring("  RCX: 0x"); print_hex(frame->rcx);
    console_writestring("  RDX: 0x"); print_hex(frame->rdx);
    console_putchar('\n');
    console_writestring("  RSI: 0x"); print_hex(frame->rsi);
    console_writestring("  RDI: 0x"); print_hex(frame->rdi);
    console_writestring("  RBP: 0x"); print_hex(frame->rbp);
    console_putchar('\n');
    console_writestring("  R8:  0x"); print_hex(frame->r8);
    console_writestring("  R9:  0x"); print_hex(frame->r9);
    console_writestring("  R10: 0x"); print_hex(frame->r10);
    console_writestring("  R11: 0x"); print_hex(frame->r11);
    console_putchar('\n');
    console_writestring("  R12: 0x"); print_hex(frame->r12);
    console_writestring("  R13: 0x"); print_hex(frame->r13);
    console_writestring("  R14: 0x"); print_hex(frame->r14);
    console_writestring("  R15: 0x"); print_hex(frame->r15);
    console_putchar('\n');
    console_writestring("  CS:  0x"); print_hex(frame->cs);
    console_writestring("  SS:  0x"); print_hex(frame->ss);
    console_putchar('\n');

    console_writestring("System halted.\n");

    while(1) {
        __asm__ volatile ("cli; hlt");
    }
}