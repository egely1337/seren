#include <nucleus/console.h>
#include <nucleus/interrupt.h>
#include <nucleus/io.h>
#include <stdbool.h>
#include <stdint.h>

#define SCANCODE_MAX 88

#define KBD_BUFFER_SIZE 256
static char g_kbd_buffer[KBD_BUFFER_SIZE];
static volatile uint32_t g_kbd_buffer_head = 0;
static volatile uint32_t g_kbd_buffer_tail = 0;

static const char g_scancode_map_lower[SCANCODE_MAX] = {
    0,   27,   '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,    0,   0,   0};

static const char g_scancode_map_upper[SCANCODE_MAX] = {
    0,   27,   '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,   0,   0,   0};

#define SCANCODE_LSHIFT_MAKE   0x2A
#define SCANCODE_LSHIFT_BREAK  0xAA
#define SCANCODE_RSHIFT_MAKE   0x36
#define SCANCODE_RSHIFT_BREAK  0xB6
#define SCANCODE_CAPSLOCK_MAKE 0x3A

static bool g_shift_pressed = false;
static bool g_capslock_on = false;

static void keyboard_irq_handler(irq_context_t *context
                                 __attribute__((unused))) {
    uint8_t scancode = inb(0x60);

    switch (scancode) {
    case SCANCODE_LSHIFT_MAKE:
    case SCANCODE_RSHIFT_MAKE:
        g_shift_pressed = true;
        return;
    case SCANCODE_LSHIFT_BREAK:
    case SCANCODE_RSHIFT_BREAK:
        g_shift_pressed = false;
        return;
    case SCANCODE_CAPSLOCK_MAKE:
        g_capslock_on = !g_capslock_on;
        return;
    }

    if (scancode >= SCANCODE_MAX || scancode >= 0x80) {
        return;
    }

    char character;
    if (g_shift_pressed) {
        character = g_scancode_map_upper[scancode];
    } else {
        character = g_scancode_map_lower[scancode];
    }

    if (g_capslock_on) {
        if (character >= 'a' && character <= 'z') {
            character -= 32;
        } else if (character >= 'A' && character <= 'Z') {
            character += 32;
        }
    }

    if (character != 0) {
        uint32_t next_head = (g_kbd_buffer_head + 1) % KBD_BUFFER_SIZE;
        if (next_head != g_kbd_buffer_tail) {
            g_kbd_buffer[g_kbd_buffer_head] = character;
            g_kbd_buffer_head = next_head;
        }
    }
}

void keyboard_init(void) {
    interrupt_register_irq_handler(1, keyboard_irq_handler);
}

char keyboard_getchar(void) {
    while (g_kbd_buffer_head == g_kbd_buffer_tail) {
        __asm__ volatile("hlt");
    }

    char c = g_kbd_buffer[g_kbd_buffer_tail];
    g_kbd_buffer_tail = (g_kbd_buffer_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}