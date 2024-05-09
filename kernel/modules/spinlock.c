#include "spinlock.h"

#define DEBUG 1

#include "../cpu/panic.h"
#include "../debug.h"

#define MAX_SPINLOCK 16

static spinlock locks[MAX_SPINLOCK];
static volatile int lock_index = 0;

spinlock *spinlock_init(void) {
    if (lock_index >= MAX_SPINLOCK) {
        panic("too many locks");
    }

    spinlock *lock = &locks[lock_index++];
    lock->locked = 0;
    return lock;
}

void spinlock_lock(spinlock *lock) {
    spinlock_test_and_set(&lock->locked, 1);
}

void spinlock_unlock(spinlock *lock) {
    spinlock_release(&lock->locked);
}
