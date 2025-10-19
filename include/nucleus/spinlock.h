#ifndef _NUCLEUS_SPINLOCK_H
#define _NUCLEUS_SPINLOCK_H

#include <asm/irqflags.h>
#include <asm/spinlock.h>

/**
 * spin_init - Initialize a spinlock to the unlocked state.
 * @lock: The spinlock to be initialized.
 */
static inline void spin_init(spinlock_t *lock) { arch_spin_init(lock); }

/**
 * spin_lock - Acquire a spinlock.
 * @lock: The spinlock to acquire.
 *
 * This function DOES NOT disable interrupts. For interrupt safety use
 * spin_lock_irqsave().
 */
static inline void spin_lock(spinlock_t *lock) { arch_spin_lock(lock); }

/**
 * spin_unlock - Release a previously acquired spinlock.
 * @lock: The spinlock to release.
 */
static inline void spin_unlock(spinlock_t *lock) { arch_spin_unlock(lock); }

/**
 * spin_lock_irqsave - Acquire a lock and save/disable local interrupts.
 * @lock: The spinlock to acquire.
 * @flags: A variable to save the current interrupt flags.
 */
#define spin_lock_irqsave(lock, flags)                                         \
	do {                                                                   \
		flags = local_irq_save();                                      \
		spin_lock(lock);                                               \
	} while (0)

/**
 * spin_unlock_irqrestore - Release a lock and restore local interrupt state.
 * @lock:   The spinlock to release.
 * @flags:  The interrupt flags saved by spin_lock_irqsave().
 */
#define spin_unlock_irqrestore(lock, flags)                                    \
	do {                                                                   \
		spin_unlock(lock);                                             \
		local_irq_restore(flags);                                      \
	} while (0)

#endif // _NUCLEUS_SPINLOCK_H