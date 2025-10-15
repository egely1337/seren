#ifndef NUCLEUS_CONSOLE_H
#define NUCLEUS_CONSOLE_H

#include <lib/stddef.h>
#include <lib/stdint.h>

/**
 * This is the main public header for Nucleus's console.
 */

#define COLOR_BLACK         0x00000000
#define COLOR_RED           0x00FF0000
#define COLOR_GREEN         0x0000FF00
#define COLOR_YELLOW        0x00FFFF00
#define COLOR_BRIGHT_RED    0x00FF5555
#define COLOR_BRIGHT_YELLOW 0x00FFFF55
#define COLOR_BRIGHT_GREEN  0x0055FF55
#define COLOR_BRIGHT_BLUE   0x005555FF
#define COLOR_LIGHT_GREY    0x00D0D0D0
#define COLOR_DARK_GREY     0x00808080

// Let's set up some default colors for our console.
// The 0x00 at the start is for Alpha.
#define CONSOLE_DEFAULT_FG_COLOR COLOR_LIGHT_GREY
#define CONSOLE_DEFAULT_BG_COLOR COLOR_BLACK

/**
 * @brief Initializes the console system.
 *
 * It gets the framebuffer ready, loads our font,
 * figures out how many rows/columns we have and gives the screen a fresh wipe.
 */
void console_init(void);

/**
 * @brief Wipes the console.
 *
 * This function will fill the entire screen with the current default background
 * color and send the cursor back to the top-left corner (0,0).
 */
void console_clear(void);

/**
 * @brief Scrolls the console.
 */
void console_scroll();

/**
 * @brief Writes a single character onto the screen.
 *
 * This is for printing one character at a time. It'll use the
 * default colors.
 *
 * @param c The character you want to put
 */
void console_putchar(char c);

/**
 * @brief Prints a whole string of characters using default colors.
 *
 * Give it a null-terminated string and it'll print it out character by
 * character using console_putchar.
 *
 * @param str The string of text you want to display
 */
void console_writestring(const char *str);

/**
 * @brief Sets the active foreground color.
 */
void console_set_fg_color(uint32_t color);

/**
 * @brief Resets the foreground color to the default.
 */
void console_reset_fg_color(void);

/**
 * @brief Kernel's high-level logging entry point.
 */
void console_log(int level, const char *message);

#endif // NUCLEUS_CONSOLE_H