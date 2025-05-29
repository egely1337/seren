#include <stddef.h>
#include <stdint.h>
#include "psf.h"

static int current_font_char_height = 0;
static int current_font_char_width = 8;
static int current_font_num_glyphs = 0;
static int current_font_bytes_per_glyph = 0;
static const uint8_t *current_font_glyph_data = NULL;
static int font_loaded_successfully = 0;

extern const unsigned char _binary_resources_font_psf_start[];
extern const unsigned char _binary_resources_font_psf_end[];

int psf_font_load(void) {
    const unsigned char *font_data_start_ptr = (const unsigned char *)&_binary_resources_font_psf_start;
    const unsigned char *font_data_end_ptr = (const unsigned char *)&_binary_resources_font_psf_end;

    size_t font_data_size = font_data_end_ptr - font_data_start_ptr;

    if (font_data_size < sizeof(struct psf1_header)) {
        font_loaded_successfully = 0;
        return -1;
    }

    const struct psf1_header *header = (const struct psf1_header *)font_data_start_ptr;

    if (header->magic[0] != PSF1_MAGIC0 || header->magic[1] != PSF1_MAGIC1) {
        font_loaded_successfully = 0;
        return -1;
    }

    current_font_char_height = header->charsize;
    current_font_bytes_per_glyph = header->charsize;

    if (header->mode > PSF1_MAXMODE) {
        current_font_num_glyphs = 256;
    } else {
        if (header->mode & PSF1_MODE512) {
            current_font_num_glyphs = 512;
        } else {
            current_font_num_glyphs = 256;
        }
    }

    current_font_glyph_data = font_data_start_ptr + sizeof(struct psf1_header);

    size_t expected_glyph_data_size = current_font_num_glyphs * current_font_bytes_per_glyph;

    if ((current_font_glyph_data + expected_glyph_data_size) > font_data_end_ptr) {
        font_loaded_successfully = 0;
        current_font_glyph_data = NULL;
        return -1;
    }

    // TODO: If PSF1_MODEHASTAB is set, handle the Unicode sequence table here.
    // For now, we assume direct mapping of first 256/512 glyphs to char codes.

    font_loaded_successfully = 1;
    return 0;
}

const unsigned char *get_psf_font_glyph_data(unsigned char c) {
    if (!font_loaded_successfully || c >= current_font_num_glyphs || current_font_glyph_data == NULL) {
        return NULL;
    }

    return current_font_glyph_data + (c * current_font_bytes_per_glyph);
}

int get_psf_font_char_height(void) {
    return current_font_char_height;
}

int get_psf_font_char_width(void) {
    return current_font_char_width;
}

int is_psf_font_loaded(void) {
    return font_loaded_successfully;
}
