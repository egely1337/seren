// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_FONT_H
#define _SEREN_FONT_H

#include <seren/stddef.h>
#include <seren/types.h>

typedef struct font_glyph {
	const u8 *bitmap;
	unsigned int width;
	unsigned int height;
	unsigned int stride;
} font_glyph_t;

typedef struct kernel_font {
	char name[16];
	unsigned int width;
	unsigned int height;
	unsigned int nr_glyphs;

	int (*get_glyph)(struct kernel_font *font, unsigned char c,
			 font_glyph_t *out_glyph);

	void *priv;
} kernel_font_t;

int font_init_psf(kernel_font_t *font, const void *data, size_t size);

static inline int font_get_glyph(kernel_font_t *font, unsigned char c,
				 font_glyph_t *out_glyph) {
	if (font && font->get_glyph)
		return font->get_glyph(font, c, out_glyph);
	return -1;
}

static inline unsigned int font_width(kernel_font_t *font) {
	return font ? font->width : 0;
}

static inline unsigned int font_height(kernel_font_t *font) {
	return font ? font->height : 0;
}

#endif // _SEREN_FONT_H