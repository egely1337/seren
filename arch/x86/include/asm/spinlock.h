#ifndef _ASM_X86_64_SPINLOCK_H
#define _ASM_X86_64_SPINLOCK_H

#include <asm/processor.h>

typedef struct {
	volatile int locked;
} spinlock_t;

#define SPIN_LOCK_UNLOCKED {0}

static inline void arch_spin_init(spinlock_t *lock) { lock->locked = 0; }

static inline void arch_spin_lock(spinlock_t *lock) {
	while (1) {
		int expected = 0;
		if (__atomic_compare_exchange_n(&lock->locked, &expected, 1, 0,
						__ATOMIC_ACQUIRE,
						__ATOMIC_RELAXED)) {
			break;
		}

		while (__atomic_load_n(&lock->locked, __ATOMIC_RELAXED)) {
			cpu_relax();
		}
	}
}

static inline void arch_spin_unlock(spinlock_t *lock) {
	__atomic_store_n(&lock->locked, 0, __ATOMIC_RELEASE);
}

#endif // _ASM_X86_64_SPINLOCK_H