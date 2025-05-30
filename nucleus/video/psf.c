#include <stddef.h>
#include <stdint.h>
#include <nucleus/font.h>
#include "psf.h"

static int current_font_char_height = 0;
static int current_font_char_width = 8;
static int current_font_num_glyphs = 0;
static int current_font_bytes_per_glyph = 0;
static const uint8_t *current_font_glyph_data = NULL;
static int font_loaded_successfully = 0;

extern const unsigned char _binary_resources_font_psf_start[];
extern const unsigned char _binary_resources_font_psf_end[];

static int psf_get_glyph_from_internal_data(const psf_font_internal_data_t *psf_data,
                                            unsigned char c,
                                            font_glyph_t *out_glyph) {
    if (!psf_data || !out_glyph) {
        return -1;
    }

    unsigned char char_to_render = c;
    if (c >= psf_data->num_glyphs) {
        if ('?' >= psf_data->num_glyphs) {
            return -2;
        }
        char_to_render = '?';
    }

    out_glyph->bitmap = psf_data->glyph_bitmaps_start + (char_to_render * psf_data->bytes_per_glyph);
    out_glyph->width_px = psf_data->char_width_px;
    out_glyph->height_px = psf_data->char_height_px;
    out_glyph->bytes_per_row = (psf_data->char_width_px + 7) / 8;

    return 0;
}

static int psf_font_interface_get_glyph(kernel_font_t *font, unsigned char c, font_glyph_t *out_glyph) {
    if (!font || !font->font_private_data) {
        return -1;
    }

    psf_font_internal_data_t *psf_internal_data = (psf_font_internal_data_t *)font->font_private_data;
    return psf_get_glyph_from_internal_data(psf_internal_data, c, out_glyph);
}

int font_init_psf(kernel_font_t *font, const void *psf_binary_start_addr, size_t psf_binary_size) {
    if (!font || !psf_binary_start_addr || psf_binary_size < sizeof(psf1_header_t)) {
        return -1;
    }
    const unsigned char *data_ptr = (const unsigned char*)psf_binary_start_addr;
    const psf1_header_t *header = (const psf1_header_t *)data_ptr;

    if (header->magic[0] != PSF1_MAGIC0 || header->magic[1] != PSF1_MAGIC1) {
        return -2;
    }

    unsigned int height = header->charsize;
    unsigned int width = 8;
    unsigned int num_glyphs = (header->mode & 0x01) ? 512 : 256;
    unsigned int bytes_per_glyph = header->charsize;
    const unsigned char *glyph_bitmaps_start = data_ptr + sizeof(psf1_header_t);

    size_t expected_total_glyph_data_size = num_glyphs * bytes_per_glyph;
    if ((sizeof(psf1_header_t) + expected_total_glyph_data_size) > psf_binary_size) {
        return -3;
    }

    static psf_font_internal_data_t S_loaded_psf_font_data;

    S_loaded_psf_font_data.raw_data_start = data_ptr;
    S_loaded_psf_font_data.raw_data_size = psf_binary_size;
    S_loaded_psf_font_data.header = header;
    S_loaded_psf_font_data.glyph_bitmaps_start = glyph_bitmaps_start;
    S_loaded_psf_font_data.num_glyphs = num_glyphs;
    S_loaded_psf_font_data.char_width_px = width;
    S_loaded_psf_font_data.char_height_px = height;
    S_loaded_psf_font_data.bytes_per_glyph = bytes_per_glyph;

    font->name[0] = 'P'; font->name[1] = 'S'; font->name[2] = 'F'; font->name[3] = '1'; font->name[4] = '\0';

    font->char_cell_width_px = width;
    font->char_cell_height_px = height;
    font->num_glyphs = num_glyphs;
    font->get_glyph = psf_font_interface_get_glyph;
    font->font_private_data = &S_loaded_psf_font_data;

    return 0;
}

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
