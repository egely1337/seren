// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <lib/format.h>
#include <lib/string.h>
#include <seren/console.h>
#include <seren/init.h>
#include <seren/pit.h>
#include <seren/printk.h>
#include <seren/spinlock.h>
#include <seren/vc.h>

#define COLOR_BLACK	    0x00000000
#define COLOR_RED	    0x00FF0000
#define COLOR_BRIGHT_RED    0x00FF5555
#define COLOR_BRIGHT_YELLOW 0x00FFFF55
#define COLOR_BRIGHT_GREEN  0x0055FF55
#define COLOR_BRIGHT_BLUE   0x005555FF
#define COLOR_LIGHT_GREY    0x00D0D0D0

#define CONSOLE_DEFAULT_FG COLOR_LIGHT_GREY
#define CONSOLE_DEFAULT_BG COLOR_BLACK
#define TAB_STOP_WIDTH	   4

static spinlock_t console_lock = SPIN_LOCK_UNLOCKED;
static const struct vc_ops *ops;
static struct vc_info info;

static int cursor_x = 0;
static int cursor_y = 0;
static u32 fg_color = CONSOLE_DEFAULT_FG;
static u32 bg_color = CONSOLE_DEFAULT_BG;
static bool initialized = false;

static u32 level_colors[] = {
    [LOGLEVEL_EMERG] = COLOR_BRIGHT_RED,
    [LOGLEVEL_ALERT] = COLOR_BRIGHT_RED,
    [LOGLEVEL_CRIT] = COLOR_BRIGHT_RED,
    [LOGLEVEL_ERR] = COLOR_RED,
    [LOGLEVEL_WARN] = COLOR_BRIGHT_YELLOW,
    [LOGLEVEL_NOTICE] = COLOR_BRIGHT_GREEN,
    [LOGLEVEL_INFO] = COLOR_LIGHT_GREY,
    [LOGLEVEL_DEBUG] = COLOR_BRIGHT_BLUE,
};

static const char *level_names[] = {
    [LOGLEVEL_EMERG] = "EMRG", [LOGLEVEL_ALERT] = "ALRT",
    [LOGLEVEL_CRIT] = "CRIT",  [LOGLEVEL_ERR] = "ERR ",
    [LOGLEVEL_WARN] = "WARN",  [LOGLEVEL_NOTICE] = "NOTE",
    [LOGLEVEL_INFO] = "INFO",  [LOGLEVEL_DEBUG] = "DBG ",
};

static void __console_scroll(void) { ops->scroll(1, bg_color); }

static void __console_putchar(char c) {
	if (cursor_y >= (int)info.rows) {
		__console_scroll();
		cursor_y = info.rows - 1;
	}

	switch (c) {
	case '\n':
		cursor_x = 0;
		cursor_y++;
		break;
	case '\b':
		if (cursor_x > 0) {
			cursor_x--;
		} else if (cursor_y > 0) {
			cursor_y--;
			cursor_x = info.cols - 1;
		}
		ops->putc(' ', cursor_y, cursor_x, bg_color, bg_color);
		break;
	case '\t':
		for (int i = 0;
		     i < (TAB_STOP_WIDTH - (cursor_x % TAB_STOP_WIDTH)); i++)
			__console_putchar(' ');
		break;
	default:
		if (cursor_x >= (int)info.cols) {
			cursor_x = 0;
			cursor_y++;
			if (cursor_y >= (int)info.rows) {
				__console_scroll();
				cursor_y = info.rows - 1;
			}
		}
		ops->putc(c, cursor_y, cursor_x, fg_color, bg_color);
		cursor_x++;
		break;
	}

	if (cursor_y >= (int)info.rows) {
		__console_scroll();
		cursor_y = info.rows - 1;
	}
}

static void tty_console_write(const char *buf, unsigned int len) {
	if (!initialized)
		return;
	u64 flags;
	spin_lock_irqsave(&console_lock, flags);
	for (unsigned int i = 0; i < len; i++)
		__console_putchar(buf[i]);
	spin_unlock_irqrestore(&console_lock, flags);
}

void console_clear(void) {
	if (!initialized)
		return;
	u64 flags;
	spin_lock_irqsave(&console_lock, flags);
	ops->clear(bg_color);
	cursor_x = 0;
	cursor_y = 0;
	spin_unlock_irqrestore(&console_lock, flags);
}

void console_log(int level, const char *message) {
	if (!initialized)
		return;
	u64 flags, ts, sec, ms;
	char time_buf[16];
	ts = timer_get_uptime_ms();
	sec = ts / 1000;
	ms = ts % 1000;
	ksnprintf(time_buf, sizeof(time_buf), "[%5lu.%03lu] ", sec, ms);

	const char *prefix =
	    (level <= LOGLEVEL_DEBUG) ? level_names[level] : "INFO";
	u32 color = (level <= LOGLEVEL_DEBUG) ? level_colors[level]
					      : CONSOLE_DEFAULT_FG;

	spin_lock_irqsave(&console_lock, flags);
	u32 orig_color = fg_color;

	fg_color = CONSOLE_DEFAULT_FG;
	for (char *p = time_buf; *p; p++)
		__console_putchar(*p);
	__console_putchar('[');
	fg_color = color;
	for (const char *p = prefix; *p; p++)
		__console_putchar(*p);
	fg_color = CONSOLE_DEFAULT_FG;
	__console_putchar(']');
	__console_putchar(' ');

	for (const char *p = message; *p; p++)
		__console_putchar(*p);
	fg_color = orig_color;
	spin_unlock_irqrestore(&console_lock, flags);
}

static struct console tty_console = {
    .name = "tty",
    .write = tty_console_write,
};

void console_set_driver(const struct vc_ops *driver_ops) {
	if (initialized || !driver_ops || !driver_ops->init)
		return;

	ops = driver_ops;
	if (ops->init(&info) != 0) {
		ops = NULL;
		return;
	}

	initialized = true;
	console_clear();
	register_console(&tty_console);
}