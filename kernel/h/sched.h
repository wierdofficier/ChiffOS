#ifndef SCHEDULE_H
#define SCHEDULE_H
#include <types.h>
#include <fs.h>
#include <list.h>

extern ring_t* task_queue;

void scheduler_install();
task_t* get_current_task();
void insert_current_task(task_t* task);

#endif
