// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <limine.h>
#include <seren/gfx.h>
#include <seren/stddef.h>
#include <seren/types.h>

static void limine_fb_put_pixel(gfx_device_t *dev, u64 x, u64 y, u32 color) {
	if (unlikely(!dev || !dev->address || x >= dev->width_px ||
		     y >= dev->height_px)) {
		return;
	}

	if (dev->bpp == 32) {
		volatile u32 *fb_ptr = (u32 *)dev->address;
		fb_ptr[y * (dev->pitch / 4) + x] = color;
	}

	// TODO: Handle other BPP values using mask_shift/size from
	// limine_framebuffer if needed
}

static void limine_fb_clear_screen(gfx_device_t *dev, u32 color) {
	if (unlikely(!dev || !dev->address)) {
		return;
	}
	for (u64 y = 0; y < dev->height_px; y++) {
		for (u64 x = 0; x < dev->width_px; x++) {
			limine_fb_put_pixel(
			    dev, x, y, color); // Use the device's put_pixel
		}
	}
}

int gfx_init_from_limine_fb(gfx_device_t *dev, struct limine_framebuffer *fb) {
	if (unlikely(!dev || !fb || !fb->address)) {
		return -1;
	}

	dev->address = fb->address;
	dev->width_px = fb->width;
	dev->height_px = fb->height;
	dev->pitch = fb->pitch;
	dev->bpp = fb->bpp;

	dev->put_pixel = limine_fb_put_pixel;
	dev->clear_screen = limine_fb_clear_screen;

	dev->private_data = (void *)fb;

	return 0;
}
