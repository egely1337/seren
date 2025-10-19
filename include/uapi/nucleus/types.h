// SPDX-License-Identifier: Apache-2.0

#ifndef _UAPI_NUCLEUS_TYPES_H
#define _UAPI_NUCLEUS_TYPES_H

#include <asm/types.h>

#ifndef __KERNEL__
#warning "You are trying to use kernel headers from user space."
#endif

typedef signed int __s_pid_t;
typedef unsigned long long __s_size_t;

#endif // _UAPI_NUCLEUS_TYPES_H