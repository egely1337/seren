// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 *
 * This file provides a abstraction for enabling and disabling IRQs. For now
 * it's just a wrapper around the legacy PIC driver. If we were to add support
 * for APIC this is where the logic would go to decide which interrupt
 * controller to talk to.
 */

#include <pic.h>
#include <seren/interrupt.h>
#include <seren/types.h>

/**
 * enable_irq - Enable a hardware interrupt line.
 * @irq: The IRQ number to enable (0-15).
 */
void enable_irq(u32 irq) { pic_unmask_irq(irq); }

/**
 * disable_irq - Disable a hardware interrupt line.
 * @irq: The IRQ number to disable (0-15).
 */
void disable_irq(u32 irq) { pic_mask_irq(irq); }
