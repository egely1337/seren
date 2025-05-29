#include <nucleus/console.h>
#include "psf.h"
#include <limine.h>

#define CONSOLE_READY (console_initialized && fb && fb->address)

extern volatile struct limine_framebuffer_request framebuffer_request;

static struct limine_framebuffer *fb = NULL;
static int cursor_x = 0;
static int cursor_y = 0;
static uint32_t current_fg_color = CONSOLE_DEFAULT_FG_COLOR;
static uint32_t current_bg_color = CONSOLE_DEFAULT_BG_COLOR;

static uint32_t char_width = 0;
static uint32_t char_height = 0;

static uint32_t screen_cols = 0;
static uint32_t screen_rows = 0;

static int console_initialized = 0;

static void put_pixel_internal(uint64_t x, uint64_t y, uint32_t color) {
    if (!CONSOLE_READY || x >= fb->width || y >= fb->height) {
        return;
    }

    if (fb->bpp == 32) {
        volatile uint32_t *fb_ptr = (uint32_t*)fb->address;
        fb_ptr[y * (fb->pitch / 4) + x] = color;
    }
}

static void draw_char_internal(char c, int x_pos_char, int y_pos_char, uint32_t fg, uint32_t bg) {
    if(!is_psf_font_loaded() || !CONSOLE_READY) {
        return;
    }

    const unsigned char *glyph_bitmap = get_psf_font_glyph_data((unsigned char)c);

    if (!glyph_bitmap) {
        glyph_bitmap = get_psf_font_glyph_data('?');
        if (!glyph_bitmap) return;
    }

    int screen_pixel_x_start = x_pos_char * char_width;
    int screen_pixel_y_start = y_pos_char * char_height;

    for (unsigned int y = 0; y < char_height; y++) {
        unsigned char row_byte = glyph_bitmap[y]; 
        for (unsigned int x = 0; x < char_width; x++) {
            if ((row_byte >> (char_width - 1 - x)) & 1) { 
                put_pixel_internal(screen_pixel_x_start + x, screen_pixel_y_start + y, fg);
            } else {
                put_pixel_internal(screen_pixel_x_start + x, screen_pixel_y_start + y, bg);
            }
        }
    }
}

void console_init(void) {
    if (console_initialized) {
        return;
    }

    psf_font_load();

    if (!is_psf_font_loaded()) {
        return;
    }

    char_width = get_psf_font_char_width();
    char_height = get_psf_font_char_height();

    if (char_width <= 0 || char_height <= 0) {
        return;
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        return;
    }

    fb = framebuffer_request.response->framebuffers[0];

    if (fb == NULL || fb->address == NULL) {
        fb = NULL;
        return;
    }

    screen_cols = fb->width / char_width;
    screen_rows = fb->height / char_height;

    console_initialized = 1;
    console_clear();
}

void console_clear(void) {
    if (!CONSOLE_READY) {
        return;
    }

    for (uint64_t y = 0; y < fb->height; y++) {
        for(uint64_t x = 0; x < fb->width; x++) {
            put_pixel_internal(x, y, current_bg_color);
        }
    }

    cursor_x = 0;
    cursor_y = 0;
}

void console_putchar_colored(char c, uint32_t fg_color, uint32_t bg_color) {
    if (!CONSOLE_READY) {
        return;
    }

    if (cursor_x >= (int)screen_cols && screen_cols > 0) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= (int)screen_rows && screen_rows > 0) {
        // TODO: scroll
        cursor_y = screen_rows > 0 ? screen_rows - 1 : 0;
    }

    draw_char_internal(c, cursor_x, cursor_y, fg_color, bg_color);
    cursor_x++;
}

void console_putchar(char c) {
    console_putchar_colored(c, current_fg_color, current_bg_color);
}

void console_writestring(const char *str) {
    if (!str || !CONSOLE_READY) return;
    for (size_t i = 0; str[i] != 0; i++) {
        console_putchar(str[i]);
    }
}
