#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "process.h"

typedef struct semaphore_t {
    int count;
    process_t *queue;
} semaphore_t;

semaphore_t *semaphore_init(void);

void semaphore_down(semaphore_t *s, process_t *p);

void semaphore_up(semaphore_t *s);

#endif // SEMAPHORE_H
