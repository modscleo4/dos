#include "task.h"

#include <stdlib.h>

task_t *queue = NULL;

void task_push(task_t *task) {
    task_t *current = queue;
    if (!current) {
        queue = task;
        return;
    }

    while (current->next) {
        current = current->next;
    }

    current->next = task;
}

task_t *task_pop(void) {
    task_t *task = queue;
    if (task) {
        queue = task->next;
    }

    return task;
}

void task_execute(task_t *task) {
    if (task && task->callback && task->process) {
        task->process->r.eax = task->callback(
            task->process,
            task->process->r.ebx,
            task->process->r.ecx,
            task->process->r.edx,
            task->process->r.esi,
            task->process->r.edi,
            task->process->r.ebp
        );
        task->process->state = PROCESS_READY;
        task->process->suspended = false;
    }
}

void task_remove_all_from(process_t *p) {
    task_t *prev = NULL;
    for (task_t *node = queue; node; node = node->next) {
        if (node->process == p) {
            if (prev) {
                prev->next = node->next;
            } else {
                queue = node->next;
            }

            free(node);
        }

        prev = node;
    }
}
