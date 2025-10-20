// SPDX-License-Identifier: Apache-2.0

#ifndef GFX_H
#define GFX_H

#include <limine.h>
#include <seren/types.h>

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
 * @brief Initializes a graphics device using a Limine framebuffer
 *
 * @param dev Pointer to the gfx_device_t structure to initialize.
 * @param fb Pointer to the limine_framebuffer provided by the bootloader.
 * @return 0 on success, -1 on error
 */
int gfx_init_from_limine_fb(gfx_device_t *dev, struct limine_framebuffer *fb);

static inline void gfx_put_pixel(gfx_device_t *dev, u64 x, u64 y, u32 color) {
	if (dev && dev->put_pixel)
		dev->put_pixel(dev, x, y, color);
}

static inline void gfx_clear_screen(gfx_device_t *dev, u32 color) {
	if (dev && dev->clear_screen)
		dev->clear_screen(dev, color);
}

static inline u64 gfx_get_width(gfx_device_t *dev) {
	return dev ? dev->width_px : 0;
}

static inline u64 gfx_get_height(gfx_device_t *dev) {
	return dev ? dev->height_px : 0;
}

#endif // GFX_H