// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <lib/format.h>
#include <lib/string.h>
#include <limine.h>
#include <seren/console.h>
#include <seren/font.h>
#include <seren/gfx.h>
#include <seren/init.h>
#include <seren/pit.h>
#include <seren/printk.h>
#include <seren/spinlock.h>

#define COLOR_BLACK	    0x00000000
#define COLOR_RED	    0x00FF0000
#define COLOR_GREEN	    0x0000FF00
#define COLOR_YELLOW	    0x00FFFF00
#define COLOR_BRIGHT_RED    0x00FF5555
#define COLOR_BRIGHT_YELLOW 0x00FFFF55
#define COLOR_BRIGHT_GREEN  0x0055FF55
#define COLOR_BRIGHT_BLUE   0x005555FF
#define COLOR_LIGHT_GREY    0x00D0D0D0
#define COLOR_DARK_GREY	    0x00808080

#define CONSOLE_DEFAULT_FG_COLOR COLOR_LIGHT_GREY
#define CONSOLE_DEFAULT_BG_COLOR COLOR_BLACK

#define CONSOLE_READY  (console_initialized_flag && active_gfx_device.address)
#define TAB_STOP_WIDTH 4

extern const unsigned char _binary_font_psf_start[];
extern const unsigned char _binary_font_psf_end[];

extern volatile struct limine_framebuffer_request framebuffer_request;

static spinlock_t console_lock = SPIN_LOCK_UNLOCKED;

static gfx_device_t active_gfx_device;
static kernel_font_t active_font;

static int cursor_x = 0;
static int cursor_y = 0;
static u32 current_fg_color = CONSOLE_DEFAULT_FG_COLOR;
static u32 current_bg_color = CONSOLE_DEFAULT_BG_COLOR;

static u32 screen_cols = 0;
static u32 screen_rows = 0;

static unsigned int inter_char_spacing = 1;
static unsigned int effective_char_cell_width = 0;

static int console_initialized_flag = 0;

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

static void console_render_glyph(char c, int char_cell_x, int char_cell_y,
				 u32 fg, u32 bg) {
	if (unlikely(!console_initialized_flag || !active_gfx_device.address ||
		     !active_font.get_glyph)) {
		return;
	}

	font_glyph_t glyph_info;

	if (unlikely(font_get_glyph(&active_font, (unsigned char)c,
				    &glyph_info) != 0)) {
		if (unlikely(font_get_glyph(&active_font, '?', &glyph_info) !=
			     0)) {
			return;
		}
	}

	int screen_pixel_x_cell_start = char_cell_x * effective_char_cell_width;
	int screen_pixel_y_cell_start = char_cell_y * font_height(&active_font);

	for (unsigned int y_in_glyph = 0; y_in_glyph < glyph_info.height;
	     y_in_glyph++) {
		for (unsigned int x_in_glyph = 0; x_in_glyph < glyph_info.width;
		     x_in_glyph++) {
			unsigned int byte_idx_in_row = x_in_glyph / 8;
			unsigned int bit_idx_in_byte = 7 - (x_in_glyph % 8);

			if (byte_idx_in_row >= glyph_info.stride)
				continue;

			const unsigned char *row_data_ptr =
			    glyph_info.bitmap +
			    (y_in_glyph * glyph_info.stride);

			if ((row_data_ptr[byte_idx_in_row] >> bit_idx_in_byte) &
			    1) {
				gfx_put_pixel(
				    &active_gfx_device,
				    screen_pixel_x_cell_start + x_in_glyph,
				    screen_pixel_y_cell_start + y_in_glyph, fg);
			} else {
				gfx_put_pixel(
				    &active_gfx_device,
				    screen_pixel_x_cell_start + x_in_glyph,
				    screen_pixel_y_cell_start + y_in_glyph, bg);
			}
		}

		for (unsigned int x_space = glyph_info.width;
		     x_space < effective_char_cell_width; x_space++) {
			gfx_put_pixel(&active_gfx_device,
				      screen_pixel_x_cell_start + x_space,
				      screen_pixel_y_cell_start + y_in_glyph,
				      bg);
		}
	}
}

/**
 * __console_scroll - Internal scroll worker. ASSUMES LOCK IS HELD.
 */
static void __console_scroll(void) {
	unsigned int char_height = font_height(&active_font);
	size_t line_size_bytes = active_gfx_device.pitch * char_height;
	size_t scroll_size_bytes = line_size_bytes * (screen_rows - 1);

	void *dest = active_gfx_device.address;
	void *src = (char *)dest + line_size_bytes;

	memmove(dest, src, scroll_size_bytes);

	void *last_line_start = (char *)dest + scroll_size_bytes;
	size_t pixels_in_line = line_size_bytes / 4;
	for (size_t i = 0; i < pixels_in_line; i++) {
		((u32 *)last_line_start)[i] = current_bg_color;
	}
}

/**
 * __console_putchar - Internal character output worker. ASSUMES LOCK IS HELD.
 */
static void __console_putchar(char c) {
	if (cursor_y >= (int)screen_rows) {
		__console_scroll();
		cursor_y = screen_rows - 1;
	}

	switch (c) {
	case '\n':
		cursor_x = 0;
		cursor_y++;
		break;
	case '\r':
		cursor_x = 0;
		break;
	case '\b':
		if (cursor_x > 0) {
			cursor_x--;
		} else if (cursor_y > 0) {
			cursor_y--;
			cursor_x = screen_cols - 1;
		}
		console_render_glyph(' ', cursor_x, cursor_y, current_bg_color,
				     current_bg_color);
		break;
	case '\t': {
		int spaces_to_next_tab =
		    TAB_STOP_WIDTH - (cursor_x % TAB_STOP_WIDTH);
		for (int i = 0; i < spaces_to_next_tab; i++) {
			__console_putchar(' ');
		}
		break;
	}
	default:
		if (cursor_x >= (int)screen_cols) {
			cursor_x = 0;
			cursor_y++;
			if (cursor_y >= (int)screen_rows) {
				__console_scroll();
				cursor_y = screen_rows - 1;
			}
		}

		console_render_glyph(c, cursor_x, cursor_y, current_fg_color,
				     current_bg_color);
		cursor_x++;
		break;
	}

	if (cursor_y >= (int)screen_rows) {
		__console_scroll();
		cursor_y = screen_rows - 1;
	}
}

static void tty_console_write(const char *buf, unsigned int len) {
	u64 flags;

	if (!CONSOLE_READY)
		return;

	spin_lock_irqsave(&console_lock, flags);

	for (unsigned int i = 0; i < len; i++)
		__console_putchar(buf[i]);

	spin_unlock_irqrestore(&console_lock, flags);
}

void console_clear(void) {
	if (unlikely(!CONSOLE_READY)) {
		return;
	}

	unsigned long flags;
	spin_lock_irqsave(&console_lock, flags);

	gfx_clear_screen(&active_gfx_device, current_bg_color);
	cursor_x = 0;
	cursor_y = 0;

	spin_unlock_irqrestore(&console_lock, flags);
}

void console_putchar(char c) {
	if (unlikely(!CONSOLE_READY))
		return;

	unsigned long flags;
	spin_lock_irqsave(&console_lock, flags);
	__console_putchar(c);
	spin_unlock_irqrestore(&console_lock, flags);
}

void console_writestring(const char *str) {
	if (unlikely(!str || !CONSOLE_READY))
		return;

	unsigned long flags;
	spin_lock_irqsave(&console_lock, flags);

	for (size_t i = 0; str[i] != '\0'; i++) {
		__console_putchar(str[i]);
	}

	spin_unlock_irqrestore(&console_lock, flags);
}

void console_set_fg_color(u32 color) {
	unsigned long flags;
	spin_lock_irqsave(&console_lock, flags);
	current_fg_color = color;
	spin_unlock_irqrestore(&console_lock, flags);
}

void console_reset_fg_color(void) {
	unsigned long flags;
	spin_lock_irqsave(&console_lock, flags);
	current_fg_color = CONSOLE_DEFAULT_FG_COLOR;
	spin_unlock_irqrestore(&console_lock, flags);
}

void console_log(int level, const char *message) {
	u64 flags;
	u64 ts;
	u64 sec, ms;
	char time_buf[16];
	const char *prefix;
	u32 color;

	if (!console_initialized_flag)
		return;

	ts = timer_get_uptime_ms();
	sec = ts / 1000;
	ms = ts % 1000;

	ksnprintf(time_buf, sizeof(time_buf), "[%5lu.%03lu] ", sec, ms);

	prefix = (level >= 0 && level <= LOGLEVEL_DEBUG) ? level_names[level]
							 : "INFO";
	color = (level >= 0 && level <= LOGLEVEL_DEBUG)
		    ? level_colors[level]
		    : CONSOLE_DEFAULT_FG_COLOR;

	spin_lock_irqsave(&console_lock, flags);

	u32 orig_color = current_fg_color;

	current_fg_color = CONSOLE_DEFAULT_FG_COLOR;
	for (char *p = time_buf; *p; p++)
		__console_putchar(*p);

	__console_putchar('[');
	current_fg_color = color;
	for (const char *p = prefix; *p; p++)
		__console_putchar(*p);
	current_fg_color = CONSOLE_DEFAULT_FG_COLOR;
	__console_putchar(']');
	__console_putchar(' ');

	for (const char *p = message; *p; p++)
		__console_putchar(*p);

	current_fg_color = orig_color;

	spin_unlock_irqrestore(&console_lock, flags);
}

static struct console tty_console = {
    .name = "fbcon	",
    .write = tty_console_write,
    .next = NULL,
};

static int __init setup_console(void) {
	if (unlikely(console_initialized_flag)) {
		return -1;
	}

	if (unlikely(framebuffer_request.response == NULL ||
		     framebuffer_request.response->framebuffer_count < 1)) {
		return -1;
	}

	struct limine_framebuffer *limine_fb =
	    framebuffer_request.response->framebuffers[0];

	if (unlikely(!limine_fb || !limine_fb->address)) {
		return -1;
	}

	if (unlikely(gfx_init_from_limine_fb(&active_gfx_device, limine_fb) !=
		     0)) {
		return -1;
	}

	size_t psf_data_size = (size_t)((const char *)_binary_font_psf_end -
					(const char *)_binary_font_psf_start);
	if (font_init_psf(&active_font, _binary_font_psf_start,
			  psf_data_size) != 0) {
		return -1;
	}

	unsigned int f_width = font_width(&active_font);
	unsigned int f_height = font_height(&active_font);

	if (unlikely(f_width == 0 || f_height == 0)) {
		return -1;
	}

	effective_char_cell_width = f_width + inter_char_spacing;

	if (effective_char_cell_width > 0) {
		screen_cols = gfx_get_width(&active_gfx_device) /
			      effective_char_cell_width;
	} else {
		screen_cols = 0;
	}

	if (f_height > 0) {
		screen_rows = gfx_get_height(&active_gfx_device) / f_height;
	} else {
		screen_rows = 0;
	}

	if (screen_cols == 0 || screen_rows == 0) {
		return -1;
	}

	console_initialized_flag = 1;
	console_clear();

	register_console(&tty_console);

	return 0;
}

pure_initcall(setup_console);
