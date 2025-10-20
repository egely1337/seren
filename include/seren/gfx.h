// SPDX-License-Identifier: Apache-2.0

#ifndef GFX_H
#define GFX_H

#include <limine.h>
#include <seren/types.h>

/**
 * struct gfx_device - A generic interface for a graphics framebuffer.
 * @address: The virtual address of the start of the framebuffer memory.
 * @width_px: The width of the screen in pixels.
 * @height_px: The height of the screen in pixels.
 * @pitch: The number of bytes from the start of one row to the next.
 * @bpp: Bits per pixel (e.g., 32 for true color).
 * @put_pixel: Function pointer to draw a single pixel.
 * @clear_screen: Function pointer to fill the screen with a color.
 * @private_data: A place for the actual driver to store its own state.
 *
 * This structure is an abstraction layer. It lets our console and other
 * higher-level graphics code draw to the screen without needing to know the
 * specific details of the underlying hardware or framebuffer format.
 */
typedef struct gfx_device {
	void *address;
	u64 width_px;
	u64 height_px;
	u64 pitch;
	u16 bpp;

	void (*put_pixel)(struct gfx_device *dev, u64 x, u64 y, u32 color);
	void (*clear_screen)(struct gfx_device *dev, u32 color);

	void *private_data; // Device-specific internal data
} gfx_device_t;

/**
 * gfx_init_from_limine_fb - Set up a gfx_device from a Limine framebuffer.
 * @dev: The `gfx_device_t` structure to initialize.
 * @fb: The framebuffer information given to us by the bootloader.
 *
 * This is our boot graphics driver. It takes the raw framebuffer info from
 * Limine and wires it up to our generic `gfx_device` interface.
 *
 * Returns 0 on success, or -1 on error.
 */
int gfx_init_from_limine_fb(gfx_device_t *dev, struct limine_framebuffer *fb);

/**
 * gfx_put_pixel - A safe wrapper to draw a pixel.
 */
static inline void gfx_put_pixel(gfx_device_t *dev, u64 x, u64 y, u32 color) {
	if (dev && dev->put_pixel)
		dev->put_pixel(dev, x, y, color);
}

/**
 * gfx_clear_screen - A safe wrapper to clear the screen.
 */
static inline void gfx_clear_screen(gfx_device_t *dev, u32 color) {
	if (dev && dev->clear_screen)
		dev->clear_screen(dev, color);
}

/**
 * gfx_get_width - A safe way to get the screen width.
 */
static inline u64 gfx_get_width(gfx_device_t *dev) {
	return dev ? dev->width_px : 0;
}

/**
 * gfx_get_height - A safe way to get the screen height.
 */
static inline u64 gfx_get_height(gfx_device_t *dev) {
	return dev ? dev->height_px : 0;
}

#endif // GFX_H