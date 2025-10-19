// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <drivers/pit.h>
#include <drivers/video/gfx.h>
#include <lib/format.h>
#include <lib/string.h>
#include <limine.h>
#include <nucleus/init.h>
#include <nucleus/tty/console.h>
#include <nucleus/tty/font.h>

#define CONSOLE_READY  (console_initialized && fb && fb->address)
#define TAB_STOP_WIDTH 4

extern const unsigned char _binary_font_psf_start[];
extern const unsigned char _binary_font_psf_end[];

extern volatile struct limine_framebuffer_request framebuffer_request;

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

static void console_render_glyph(char c, int char_cell_x, int char_cell_y,
				 u32 fg, u32 bg) {
	if (!console_initialized_flag || !active_gfx_device.address ||
	    !active_font.get_glyph) {
		return;
	}

	font_glyph_t glyph_info;

	if (font_get_glyph(&active_font, (unsigned char)c, &glyph_info) != 0) {
		if (font_get_glyph(&active_font, '?', &glyph_info) != 0) {
			return;
		}
	}

	int screen_pixel_x_cell_start = char_cell_x * effective_char_cell_width;
	int screen_pixel_y_cell_start =
	    char_cell_y * font_get_char_height(&active_font);

	for (unsigned int y_in_glyph = 0; y_in_glyph < glyph_info.height_px;
	     y_in_glyph++) {
		for (unsigned int x_in_glyph = 0;
		     x_in_glyph < glyph_info.width_px; x_in_glyph++) {
			unsigned int byte_idx_in_row = x_in_glyph / 8;
			unsigned int bit_idx_in_byte = 7 - (x_in_glyph % 8);

			if (byte_idx_in_row >= glyph_info.bytes_per_row)
				continue;

			const unsigned char *row_data_ptr =
			    glyph_info.bitmap +
			    (y_in_glyph * glyph_info.bytes_per_row);

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

		for (unsigned int x_space = glyph_info.width_px;
		     x_space < effective_char_cell_width; x_space++) {
			gfx_put_pixel(&active_gfx_device,
				      screen_pixel_x_cell_start + x_space,
				      screen_pixel_y_cell_start + y_in_glyph,
				      bg);
		}
	}
}

void console_init(void) {
	if (console_initialized_flag) {
		return;
	}

	if (framebuffer_request.response == NULL ||
	    framebuffer_request.response->framebuffer_count < 1) {
		return;
	}

	struct limine_framebuffer *limine_fb =
	    framebuffer_request.response->framebuffers[0];

	if (!limine_fb || !limine_fb->address) {
		return;
	}

	if (gfx_init_from_limine_fb(&active_gfx_device, limine_fb) != 0) {
		return;
	}

	size_t psf_data_size = (size_t)((const char *)_binary_font_psf_end -
					(const char *)_binary_font_psf_start);
	if (font_init_psf(&active_font, _binary_font_psf_start,
			  psf_data_size) != 0) {
		return;
	}

	unsigned int f_width = font_get_char_width(&active_font);
	unsigned int f_height = font_get_char_height(&active_font);

	if (f_width == 0 || f_height == 0) {
		return;
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
		return;
	}

	console_initialized_flag = 1;
	console_clear();
}

void console_clear(void) {
	if (!console_initialized_flag || !active_gfx_device.clear_screen) {
		return;
	}

	gfx_clear_screen(&active_gfx_device, current_bg_color);
	cursor_x = 0;
	cursor_y = 0;
}

void console_scroll(void) {
	if (!console_initialized_flag || screen_rows <= 1) {
		return;
	}

	unsigned int char_height = font_get_char_height(&active_font);
	size_t line_size_bytes = active_gfx_device.pitch * char_height;

	size_t scroll_size_bytes = line_size_bytes * (screen_rows - 1);

	void *dest = active_gfx_device.address;
	void *src =
	    (void *)((uintptr_t)active_gfx_device.address + line_size_bytes);

	memmove(dest, src, scroll_size_bytes);

	void *last_line_start =
	    (void *)((uintptr_t)active_gfx_device.address + scroll_size_bytes);

	u32 *last_line_pixels = (u32 *)last_line_start;
	size_t pixels_in_line = line_size_bytes / 4;

	for (size_t i = 0; i < pixels_in_line; i++) {
		last_line_pixels[i] = current_bg_color;
	}
}

void console_putchar_colored(char c, u32 fg_color, u32 bg_color) {
	if (!console_initialized_flag) {
		return;
	}

	if (cursor_y >= (int)screen_rows) {
		console_scroll();
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
			console_render_glyph(' ', cursor_x, cursor_y, bg_color,
					     bg_color);
		} else if (cursor_y > 0) {
			cursor_y--;
			cursor_x = (screen_cols > 0) ? (screen_cols - 1) : 0;
			console_render_glyph(' ', cursor_x, cursor_y, bg_color,
					     bg_color);
		}
		break;
	case '\t': {
		int spaces_to_next_tab =
		    TAB_STOP_WIDTH - (cursor_x % TAB_STOP_WIDTH);
		for (int i = 0; i < spaces_to_next_tab; i++) {
			console_putchar_colored(' ', fg_color, bg_color);
		}
	}
		return;
	default:
		if (cursor_x >= (int)screen_cols && screen_cols > 0) {
			cursor_x = 0;
			cursor_y++;
		}

		if (cursor_y >= (int)screen_rows && screen_rows > 0) {
			console_scroll();
			cursor_y = screen_rows - 1;
		}

		console_render_glyph(c, cursor_x, cursor_y, fg_color, bg_color);
		cursor_x++;
		break;
	}

	if (cursor_y >= (int)screen_rows && screen_rows > 0) {
		console_scroll();
		cursor_y = screen_rows - 1;
	}
}

void console_putchar(char c) {
	console_putchar_colored(c, current_fg_color, current_bg_color);
}

void console_writestring(const char *str) {
	if (!str || !console_initialized_flag)
		return;
	for (size_t i = 0; str[i] != '\0'; i++) {
		console_putchar(str[i]);
	}
}

void console_set_fg_color(u32 color) { current_fg_color = color; }

void console_reset_fg_color(void) {
	current_fg_color = CONSOLE_DEFAULT_FG_COLOR;
}

void console_log(int level, const char *message) {
	if (!console_initialized_flag)
		return;

	u64 uptime_ms = timer_get_uptime_ms();
	u64 seconds = uptime_ms / 1000;
	u64 milliseconds = uptime_ms % 1000;

	char time_buf[16];

	ksnprintf(time_buf, sizeof(time_buf), "[%5lu.%03lu] ", seconds,
		  milliseconds);

	console_reset_fg_color();
	console_writestring(time_buf);

	const char *prefix_text = NULL;
	u32 prefix_color = CONSOLE_DEFAULT_FG_COLOR;

	switch (level) {
	case 0:
		prefix_text = "EMRG";
		prefix_color = COLOR_BRIGHT_RED;
		break;
	case 1:
		prefix_text = "ALRT";
		prefix_color = COLOR_BRIGHT_RED;
		break;
	case 2:
		prefix_text = "CRIT";
		prefix_color = COLOR_BRIGHT_RED;
		break;
	case 3:
		prefix_text = "ERR ";
		prefix_color = COLOR_RED;
		break;
	case 4:
		prefix_text = "WARN";
		prefix_color = COLOR_BRIGHT_YELLOW;
		break;
	case 5:
		prefix_text = "NOTE";
		prefix_color = COLOR_BRIGHT_GREEN;
		break;
	case 7:
		prefix_text = "DBG ";
		prefix_color = COLOR_BRIGHT_BLUE;
		break;
	}

	if (prefix_text) {
		console_reset_fg_color();
		console_putchar('[');
		console_set_fg_color(prefix_color);
		console_writestring(prefix_text);
		console_reset_fg_color();
		console_writestring("] ");
	} else {
		console_writestring("       ");
	}

	console_reset_fg_color();
	console_writestring(message);
}
