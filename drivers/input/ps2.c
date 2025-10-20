// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <io.h>
#include <pic.h>
#include <seren/init.h>
#include <seren/interrupt.h>
#include <seren/tty.h>
#include <seren/types.h>

#define SCANCODE_MAX 88

static const char g_scancode_map_lower[SCANCODE_MAX] = {
    0,	 27,   '1',  '2', '3',	'4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',	'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,	  'a',	's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,	  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,	' ', 0,	  0,   0,   0,	 0,   0,   0,
    0,	 0,    0,    0,	  0,	0,   0,	  0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,	  0,	0,   0,	  0};

static const char g_scancode_map_upper[SCANCODE_MAX] = {
    0,	 27,   '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0,	  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,	  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    '*', 0,   ' ', 0,	 0,   0,   0,	0,   0,	  0,
    0,	 0,    0,    0,	  0,   0,   0,	 0,   0,   '-', 0,   0,	  0,
    '+', 0,    0,    0,	  0,   0,   0,	 0};

#define SCANCODE_LSHIFT_MAKE   0x2A
#define SCANCODE_LSHIFT_BREAK  0xAA
#define SCANCODE_RSHIFT_MAKE   0x36
#define SCANCODE_RSHIFT_BREAK  0xB6
#define SCANCODE_CAPSLOCK_MAKE 0x3A

static bool g_shift_pressed = false;
static bool g_capslock_on = false;

static void keyboard_irq_handler(struct pt_regs *context
				 __attribute__((unused))) {
	u8 scancode = inb(0x60);

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
		tty_receive_char(character);
	}
}

void keyboard_init(void) { request_irq(1, keyboard_irq_handler); }

static int __init setup_keyboard(void) {
	keyboard_init();

	pic_unmask_irq(1);

	return 0;
}

device_initcall(setup_keyboard);
