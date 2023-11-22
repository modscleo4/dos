#ifndef SPINLOCK_H
#define SPINLOCK_H

typedef struct spinlock {
    volatile int locked;
} spinlock;

extern int spinlock_test_and_set(volatile int *ptr, int val);

extern void spinlock_release(volatile int *ptr);

spinlock *spinlock_init(void);

void spinlock_lock(spinlock *lock);

void spinlock_unlock(spinlock *lock);

#endif // SPINLOCK_H
