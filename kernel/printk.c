// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <lib/format.h>
#include <lib/stdarg.h>
#include <lib/string.h>
#include <seren/log.h>
#include <seren/printk.h>
#include <seren/tty.h>
#include <seren/types.h>

#define PRINTK_BUF_SIZE 512

static int console_loglevel = LOGLEVEL_DEBUG;
static struct console *console_list = NULL;

static int __parse_level(const char **fmt) {
	const char *p = *fmt;

	if (p[0] == '<' && p[1] >= '0' && p[1] <= '7' && p[2] == '>') {
		int level = p[1] - '0';
		*fmt = p + 3;
		return level;
	}

	return LOGLEVEL_INFO;
}

static void __emit_to_consoles(int level, const char *msg) {
	struct console *con;

	if (level > console_loglevel)
		return;

	for (con = console_list; con; con = con->next) {
		if (con->write)
			con->write(msg, strlen(msg));
	}
}

int vprintk(const char *fmt, va_list args) {
	static char buf[PRINTK_BUF_SIZE];
	const char *fmt_body;
	int level;
	int len;

	if (!fmt)
		return 0;

	fmt_body = fmt;
	level = __parse_level(&fmt_body);

	len = kvsnprintf(buf, PRINTK_BUF_SIZE, fmt_body, args);
	if (len <= 0)
		return len;

	klog_write(level, buf);
	__emit_to_consoles(level, buf);

	return len;
}

int printk(const char *fmt, ...) {
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vprintk(fmt, args);
	va_end(args);

	return ret;
}

void register_console(struct console *con) {
	if (!con || !con->write)
		return;

	con->next = console_list;
	console_list = con;
}