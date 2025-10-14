#ifndef KEYBOARD_H
#define KEYBOARD_H

/**
 * @brief Initializes the PS/2 keyboard driver.
 */
void keyboard_init(void);

/**
 * @brief Reads a character.
 */
char keyboard_getchar(void);

#endif // KEYBOARD_H