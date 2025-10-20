#ifndef _ASM_X86_64_SPINLOCK_H
#define _ASM_X86_64_SPINLOCK_H

#include <asm/processor.h>

/**
 * spinlock_t - Simple spinlock implementation
 *
 * This is a basic spinlock for synchronizing access to shared data.
 * It's "unfair", so it doesn't guarantee FIFO ordering for waiters,
 * but it's simple and effective for short critical sections.
 *
 * The `volatile` keyword is important to prevent the compiler from caching
 * the lock's state in a register (this ensures we always read the latest
 * value from memory).
 */
typedef struct {
	volatile int locked;
} spinlock_t;

#define SPIN_LOCK_UNLOCKED {0}

/**
 * arch_spin_init - Initialize a spinlock to the unlocked state.
 * @lock: The spinlock to initialize.
 */
static inline void arch_spin_init(spinlock_t *lock) { lock->locked = 0; }

/**
 * arch_spin_lock - Acquire a spinlock, spinning if necessary.
 * @lock: The spinlock to acquire.
 */
static inline void arch_spin_lock(spinlock_t *lock) {
	while (1) {
		int expected = 0;
		/**
		 * Try to automatically swap `locked` from 0 to 1.
		 * `__ATOMIC_ACQUIRE` creates a memory barrier that prevents
		 * subsequent memory operations from being reordered before
		 * this point.
		 */
		if (__atomic_compare_exchange_n(&lock->locked, &expected, 1, 0,
						__ATOMIC_ACQUIRE,
						__ATOMIC_RELAXED)) {
			break;
		}

		/**
		 * The lock is held by someone else. Spin while it's locked.
		 * We use `__ATOMIC_RELAXED` here because we don't need any
		 * memory ordering guarantees while just waiting.
		 */
		while (__atomic_load_n(&lock->locked, __ATOMIC_RELAXED)) {
			cpu_relax();
		}
	}
}

/**
 * arch_spin_unlock - Release a spinlock.
 * @lock: The spinlock to release.
 */
static inline void arch_spin_unlock(spinlock_t *lock) {
	__atomic_store_n(&lock->locked, 0, __ATOMIC_RELEASE);
}

#endif // _ASM_X86_64_SPINLOCK_H