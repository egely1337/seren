// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_FONT_H
#define _SEREN_FONT_H

#include <seren/stddef.h>
#include <seren/types.h>

/**
 * struct font_glyph - Represents a single character's bitmap.
 * @bitmap: A pointer to the raw bitmap data for the character.
 * @width: The width of the character in pixels.
 * @height: The height of the character in pixels.
 * @stride: The number of bytes per row in the bitmap data.
 */
typedef struct font_glyph {
	const u8 *bitmap;
	unsigned int width;
	unsigned int height;
	unsigned int stride;
} font_glyph_t;

/**
 * struct kernel_font - A generic interface for a font.
 * @name: A simple name for the font, like "PSF1".
 * @width: The width of a character cell. For monospaced fonts this is fixed.
 * @height: The height of a character cell.
 * @nr_glyphs: The number of characters available in the font.
 * @get_glyph: A function pointer to get the bitmap for a specific character.
 * @priv: A spot for the font driver to store its own private data.
 *
 * This structure lets our console code talk to any kind of font driver
 * without needing to know the specifics of the font format (like PSF, etc.).
 */
typedef struct kernel_font {
	char name[16];
	unsigned int width;
	unsigned int height;
	unsigned int nr_glyphs;

	int (*get_glyph)(struct kernel_font *font, unsigned char c,
			 font_glyph_t *out_glyph);

	void *priv;
} kernel_font_t;

/**
 * psf_font_init - Initialize a `kernel_font` from PC Screen Font data.
 * @font: The `kernel_font_t` struct to fill out.
 * @data: A pointer to the raw PSF file data in memory.
 * @size: The size of the font data.
 *
 * This function parses a PSF font file and sets up the `kernel_font_t`
 * structure with the right dimensions and function pointers.
 */
int psf_font_init(kernel_font_t *font, const void *data, size_t size);

/**
 * font_get_glyph - A wrapper to safely get a character's glyph.
 */
static inline int font_get_glyph(kernel_font_t *font, unsigned char c,
				 font_glyph_t *out_glyph) {
	if (font && font->get_glyph)
		return font->get_glyph(font, c, out_glyph);
	return -1;
}

/**
 * font_width - A safe way to get the width of a font.
 */
static inline unsigned int font_width(kernel_font_t *font) {
	return font ? font->width : 0;
}

/**
 * font_height - A safe way to get the height of a font.
 */
static inline unsigned int font_height(kernel_font_t *font) {
	return font ? font->height : 0;
}

#endif // _SEREN_FONT_H