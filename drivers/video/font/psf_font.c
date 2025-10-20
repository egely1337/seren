// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <seren/font.h>
#include <seren/types.h>

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

struct psf1_hdr {
	u8 magic[2];
	u8 mode;
	u8 charsize;
} __attribute__((packed));

struct psf_priv {
	const u8 *data;
	size_t size;
	const struct psf1_hdr *hdr;
	const u8 *glyphs;
	unsigned int nr_glyphs;
	unsigned int width;
	unsigned int height;
	unsigned int glyph_size;
};

static int __psf_get_glyph(const struct psf_priv *priv, unsigned char c,
			   font_glyph_t *out_glyph) {
	if (!priv || !out_glyph)
		return -1;

	unsigned char char_to_render = (c < priv->nr_glyphs) ? c : '?';
	if (char_to_render == '?' && '?' >= priv->nr_glyphs)
		return -2;

	out_glyph->bitmap = priv->glyphs + (char_to_render * priv->glyph_size);
	out_glyph->width = priv->width;
	out_glyph->height = priv->height;
	out_glyph->stride = (priv->width + 7) / 8;

	return 0;
}

static int psf_get_glyph(kernel_font_t *font, unsigned char c,
			 font_glyph_t *out_glyph) {
	if (!font || !font->priv)
		return -1;

	return __psf_get_glyph(font->priv, c, out_glyph);
}

int font_init_psf(kernel_font_t *font, const void *data, size_t size) {
	if (unlikely(!font || !data || size < sizeof(struct psf1_hdr)))
		return -1;

	const struct psf1_hdr *hdr = data;
	if (hdr->magic[0] != PSF1_MAGIC0 || hdr->magic[1] != PSF1_MAGIC1)
		return -2;

	const u8 *glyphs = (const u8 *)data + sizeof(struct psf1_hdr);
	unsigned int nr_glyphs = (hdr->mode & 0x01) ? 512 : 256;
	unsigned int glyph_size = hdr->charsize;

	if ((sizeof(struct psf1_hdr) + (nr_glyphs * glyph_size)) > size)
		return -3;

	static struct psf_priv s_priv;

	s_priv.data = data;
	s_priv.size = size;
	s_priv.hdr = hdr;
	s_priv.glyphs = glyphs;
	s_priv.nr_glyphs = nr_glyphs;
	s_priv.width = 8;
	s_priv.height = hdr->charsize;
	s_priv.glyph_size = glyph_size;

	font->name[0] = 'P';
	font->name[1] = 'S';
	font->name[2] = 'F';
	font->name[3] = '1';
	font->name[4] = '\0';
	font->width = s_priv.width;
	font->height = s_priv.height;
	font->nr_glyphs = nr_glyphs;
	font->get_glyph = psf_get_glyph;
	font->priv = &s_priv;

	return 0;
}