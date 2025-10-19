// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define INIT_CALLS_LEVEL(level) KEEP(*(.initcall##level##.init))

#define INIT_CALLS                                                             \
	INIT_CALLS_LEVEL(0)                                                    \
	INIT_CALLS_LEVEL(1)                                                    \
	INIT_CALLS_LEVEL(2)                                                    \
	INIT_CALLS_LEVEL(3)                                                    \
	INIT_CALLS_LEVEL(4)                                                    \
	INIT_CALLS_LEVEL(5)                                                    \
	INIT_CALLS_LEVEL(6)
