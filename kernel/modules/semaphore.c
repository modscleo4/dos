#include "semaphore.h"

#include <stdlib.h>

static void semaphore_queue_push(semaphore_t *s, process_t *p) {
    if (s->queue == NULL) {
        s->queue = p;
        p->next = NULL;
    } else {
        process_t *q = s->queue;
        while (q->next != NULL) {
            q = q->next;
        }
        q->next = p;
        p->next = NULL;
    }
}

static process_t *semaphore_queue_pop(semaphore_t *s) {
    if (s->queue == NULL) {
        return NULL;
    }

    process_t *p = s->queue;
    s->queue = p->next;
    return p;
}

semaphore_t *semaphore_init(void) {
    semaphore_t *s = (semaphore_t *)malloc(sizeof(semaphore_t));
    s->count = 0;
    s->queue = NULL;
}

void semaphore_down(semaphore_t *s, process_t *p) {
    if (s->count > 0) {
        s->count--;
    } else {
        // Remove from ready queue and add to semaphore queue
        p->state = PROCESS_WAITING;
        process_remove_ready_queue(p);
        semaphore_queue_push(s, p);
    }
}

void semaphore_up(semaphore_t *s) {
    s->count++;

    process_t *p = semaphore_queue_pop(s);
    if (p != NULL) {
        p->state = PROCESS_READY;
        process_add_ready_queue(p);
    }
}
