// SPDX-License-Identifier: Apache-2.0

#ifndef _UAPI_ASM_X86_64_TYPES_H
#define _UAPI_ASM_X86_64_TYPES_H

/**
 * This is the architecture-specific uapi (user API) types header.
 * For x86_64 the standard fixed-width integer types are sufficent
 * so we just pull in the generic definitions. Other architectures might
 * need to define specific types here.
 */
#include <uapi/asm-generic/int-ll64.h>

#endif // _UAPI_ASM_X86_64_TYPES_H