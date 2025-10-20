#ifndef _SEREN_VC_H
#define _SEREN_VC_H

#include <seren/types.h>

struct vc_ops;

struct vc_info {
	unsigned int cols;
	unsigned int rows;
};

struct vc_ops {
	int (*init)(struct vc_info *info);

	void (*putc)(char c, int y, int x, u32 fg, u32 bg);

	void (*scroll)(unsigned int rows, u32 bg);

	void (*clear)(u32 bg);

	void (*cursor)(int y, int x, bool visible);

	void (*deinit)(void);
};

void console_set_driver(const struct vc_ops *ops);

#endif // _SEREN_VC_H