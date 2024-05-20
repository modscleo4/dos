#ifndef TASK_H
#define TASK_H

#include "process.h"

enum TaskType {
    TASK_TYPE_IO,
};

typedef struct task_t {
    enum TaskType type;
    process_t *process;
    int32_t (*callback)(process_t *p, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    struct task_t *next;
} task_t;

void task_push(task_t *task);
task_t *task_pop(void);
void task_execute(task_t *task);
void task_remove_all_from(process_t *p);

#endif // TASK_H
