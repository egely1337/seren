#ifndef _PSF_H
#define _PSF_H

#include <stdint.h>

/**
 * PSF1 Font Specifics
 * 
 * These are the magic numbers and flags that helps us understand
 * what kind of PS1 font file we're dealing with.
 */

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

#define PSF1_MODE512 0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE 0x05

#define PSF1_SEPARATOR 0xFFFF
#define PSF1_STARTSEQ 0xFFFE

struct psf1_header {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t charsize;
};

/**
 * @brief Reads our embedded PSF font data and figures out what's what.
 * 
 * This function is the workhorse. It'll look at the font data, check the
 * header and store important details like character height and where the
 * actual pixel data starts.
 * 
 * @return 0 if everyting loaded up fine
 */
int psf_font_load(void);

/**
 * @brief How wide is a character in our font, in pixels
 * (Spoiler: For PSF1, this is always 8 pixels)
 * 
 * @return The width of a character in pixels
 */
int get_psf_font_char_width(void);

/**
 * @brief How tall is a character in our font, in pixels
 * 
 * @return The height of a character in pixels
 */
int get_psf_font_char_height(void);

/**
 * @brief A quick check to see if our font actually loaded okay and is ready to use.
 * 
 * @return 1 (true) if the font is loaded, 0 (false) if not.
 */
int is_psf_font_loaded(void);

/**
 * @brief It gives you the pixel data of the character you gave
 * 
 * Takes a character (like 'A') and returns a pointer to the raw bytes
 * that make up the bitmap of that character.
 * 
 * @param c The character code
 * @return A pointer to the glyph's bitmap data, or NULL if the character isn't in our font
 */
const unsigned char *get_psf_font_glyph_data(unsigned char c);

#endif // _PSF_H