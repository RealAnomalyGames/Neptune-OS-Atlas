#include "scheduler.h"

static TaskID scheduler_last_task;

void scheduler_initialize(void)
{
    scheduler_last_task = 0;
}

TaskID scheduler_get_next_task(void)
{
    uint32_t i;
    uint32_t index;
    TaskID current_id;
    AtlasTask* task;

    current_id = scheduler_last_task;

    for (i = 1; i <= ATLAS_MAX_TASKS; i++)
    {
        index = (current_id + i) % ATLAS_MAX_TASKS;

        task = task_get(index);

        if (task != 0 && task->state == TASK_READY)
        {
            return task->id;
        }
    }

    return scheduler_last_task;
}

void scheduler_schedule(void)
{
    TaskID next_task;
    AtlasTask* current_task;
    AtlasTask* next_task_info;

    next_task = scheduler_get_next_task();

    if (next_task == scheduler_last_task)
    {
        return;
    }

    current_task = task_get_current();
    next_task_info = task_get(next_task);

    if (current_task == 0 || next_task_info == 0)
    {
        return;
    }

    /*
     * Mark the current task as ready.
     */
    task_set_ready(current_task->id);

    /*
     * Mark the next task as running.
     */
    task_set_current(next_task);

    /*
     * Remember which task is now active.
     */
    scheduler_last_task = next_task;

    /*
     * Actually switch CPU context.
     */
    task_context_switch(current_task, next_task_info);
}