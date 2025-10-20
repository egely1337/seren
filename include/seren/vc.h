// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_VC_H
#define _SEREN_VC_H

#include <seren/types.h>

/**
 * This header defines the interface between the high-level console logic (which
 * handles things like newlines and scrollin text) and the low-level graphic
 * drivers (which just know how to draw pixels).
 *
 * "VC" stands for Virtual Console.
 */

/* Forwars Declaration */
struct vc_ops;

/**
 * struct vc_info - Basic information about the console grid.
 * @cols: The number of character columns.
 * @rows: The number of character rows.
 *
 * A driver fills this out during its init sequence to tell the console
 * layer about the dimensions of the screen.
 */
struct vc_info {
	unsigned int cols;
	unsigned int rows;
};

/**
 * struct vc_ops - A set of function pointers for a console driver.
 *
 * A graphics driver (like our framebuffer console) implements these functions
 * to provide basic terminal operations. The main console logic then calls
 * these pointers to get text on the screen.
 */
struct vc_ops {
	int (*init)(struct vc_info *info);
	void (*putc)(char c, int y, int x, u32 fg, u32 bg);
	void (*scroll)(unsigned int rows, u32 bg);
	void (*clear)(u32 bg);
	void (*cursor)(int y, int x, bool visible);
	void (*deinit)(void);
};

/**
 * console_set_driver - Hooks a graphics driver into the console system.
 * @ops: A pointer to the driver's implemented `vc_ops` structure.
 *
 * This is the function a framebuffer driver (or VGA text-mode driver, etc.)
 * calls during its initcall to become the active console.
 */
void console_set_driver(const struct vc_ops *ops);

#endif // _SEREN_VC_H