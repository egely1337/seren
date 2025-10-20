// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "fbcon: " fmt

#include <lib/string.h>
#include <limine.h>
#include <seren/font.h>
#include <seren/gfx.h>
#include <seren/init.h>
#include <seren/printk.h>
#include <seren/vc.h>

extern volatile struct limine_framebuffer_request framebuffer_request;
extern const unsigned char _binary_font_psf_start[];
extern const unsigned char _binary_font_psf_end[];

static gfx_device_t fb_dev;
static kernel_font_t font;
static unsigned int inter_char_spacing = 1;
static unsigned int cell_width;

static void fbcon_putc(char c, int y, int x, u32 fg, u32 bg) {
	font_glyph_t glyph;

	if (unlikely(font_get_glyph(&font, (unsigned char)c, &glyph) != 0)) {
		if (unlikely(font_get_glyph(&font, '?', &glyph) != 0)) {
			return;
		}
	}

	int px = x * cell_width;
	int py = y * font_height(&font);

	for (unsigned int gy = 0; gy < glyph.height; gy++) {
		for (unsigned int gx = 0; gx < glyph.width; gx++) {
			unsigned int byte_idx = gx / 8;
			unsigned int bit_idx = 7 - (gx % 8);
			const u8 *row = glyph.bitmap + (gy * glyph.stride);

			if ((row[byte_idx] >> bit_idx) & 1) {
				gfx_put_pixel(&fb_dev, px + gx, py + gy, fg);
			} else {
				gfx_put_pixel(&fb_dev, px + gx, py + gy, bg);
			}
		}

		for (unsigned int sx = glyph.width; sx < cell_width; sx++) {
			gfx_put_pixel(&fb_dev, px + sx, py + gy, bg);
		}
	}
}

static void fbcon_scroll(unsigned int rows, u32 bg) {
	unsigned int char_h = font_height(&font);
	size_t line_bytes = fb_dev.pitch * char_h;
	size_t scroll_bytes = line_bytes * (fb_dev.height_px / char_h - rows);
	size_t clear_bytes = line_bytes * rows;

	void *dest = fb_dev.address;
	void *src = (char *)dest + line_bytes * rows;
	void *clear_start = (char *)dest + scroll_bytes;

	memmove(dest, src, scroll_bytes);

	u32 *clear_ptr = clear_start;
	for (size_t i = 0; i < clear_bytes / sizeof(u32); i++) {
		clear_ptr[i] = bg;
	}
}

static void fbcon_clear(u32 bg) { gfx_clear_screen(&fb_dev, bg); }

static int fbcon_init(struct vc_info *info) {
	if (unlikely(!framebuffer_request.response ||
		     framebuffer_request.response->framebuffer_count < 1)) {
		return -1;
	}
	struct limine_framebuffer *fb =
	    framebuffer_request.response->framebuffers[0];
	if (unlikely(!fb || !fb->address)) {
		return -1;
	}
	if (unlikely(gfx_init_from_limine_fb(&fb_dev, fb) != 0)) {
		return -1;
	}

	size_t psf_size =
	    (size_t)(_binary_font_psf_end - _binary_font_psf_start);
	if (psf_font_init(&font, _binary_font_psf_start, psf_size) != 0) {
		return -1;
	}

	unsigned int f_width = font_width(&font);
	unsigned int f_height = font_height(&font);
	if (unlikely(f_width == 0 || f_height == 0)) {
		return -1;
	}

	cell_width = f_width + inter_char_spacing;
	info->cols = gfx_get_width(&fb_dev) / cell_width;
	info->rows = gfx_get_height(&fb_dev) / f_height;

	if (info->cols == 0 || info->rows == 0) {
		return -1;
	}

	pr_info("initialized (%ux%u grid)\n", info->cols, info->rows);
	return 0;
}

static const struct vc_ops fbcon_ops = {
    .init = fbcon_init,
    .putc = fbcon_putc,
    .scroll = fbcon_scroll,
    .clear = fbcon_clear,
};

static int __init setup_fbcon(void) {
	console_set_driver(&fbcon_ops);
	return 0;
}

pure_initcall(setup_fbcon);