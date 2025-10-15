#ifndef TIMER_H
#define TIMER_H

#include <nucleus/types.h>

#define TIMER_IRQ       (uint8_t)0
#define TIMER_FREQUENCY (uint32_t)1193182

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
uint64_t timer_get_uptime_ms(void);

#endif