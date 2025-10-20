// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_CONSOLE_H
#define _SEREN_CONSOLE_H

struct console {
	char name[16];
	void (*write)(const char *buf, unsigned int len);
	struct console *next;
};

/**
 * register_console - Register a console device with the kernel
 * @con: The console structure to register
 */
void register_console(struct console *c);

#endif // _SEREN_CONSOLE_H