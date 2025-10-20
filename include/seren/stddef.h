// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_STDDEF_H
#define _SEREN_STDDEF_H

#undef NULL
#define NULL ((void *)0)

#define offsetof(st, m) ((size_t)&(((st *)0)->m))

#endif // _SEREN_STDDEF_H