#ifndef _NUCLEUS_STDDEF_H
#define _NUCLEUS_STDDEF_H

#undef NULL
#define NULL ((void *)0)

#define offsetof(st, m) ((size_t)&(((st *)0)->m))

#endif // _NUCLEUS_STDDEF_H