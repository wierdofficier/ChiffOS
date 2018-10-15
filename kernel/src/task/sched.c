#include <proc.h>
#include <sched.h>
#include <list.h>

ring_t* task_queue;

void scheduler_install()
{
    task_queue = ring_list_create();
}

task_t* get_current_task()
{
    task_queue->current = task_queue->current->next;
    return (task_t*)task_queue->current->data;
}

void insert_current_task(task_t* task_data)
{
    ring_list_insert(task_queue, task_data);
}
bool scheduler_shouldSwitchTask()
 	{
 	return(task_queue->begin != task_queue->begin->next);
 	}
 	
void delete_current_task(task_t* task_data)
{
    ring_list_delete_first(task_queue, task_data);
}
