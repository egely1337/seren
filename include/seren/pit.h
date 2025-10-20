// SPDX-License-Identifier: Apache-2.0

#ifndef PIT_H
#define PIT_H

#include <seren/interrupt.h>
#include <seren/types.h>

#define TIMER_IRQ	(u8)0
#define TIMER_FREQUENCY (u32)1193182

/**
 * @brief Every millisecond cpu will fire this function.
 */
void timer_handler(void);

/**
 * @brief The function that initializes timer
 */
void timer_init(void);

/**
 * @brief Gets the number of milliseconds since the timer was initialized.
 */
u64 timer_get_uptime_ms(void);

#endif