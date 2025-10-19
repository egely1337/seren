// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <nucleus/interrupt.h>
#include <nucleus/types.h>
#include <pic.h>

void enable_irq(u32 irq) { pic_unmask_irq(irq); }

void disable_irq(u32 irq) { pic_mask_irq(irq); }
