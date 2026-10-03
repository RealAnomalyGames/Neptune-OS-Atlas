#include "task.h"

static AtlasTaskManager task_manager;

static uint8_t task_stacks[ATLAS_MAX_TASKS][ATLAS_TASK_STACK_SIZE];

static void task_copy_name(char* destination, const char* source)
{
    uint32_t i;

    for (i = 0; i < ATLAS_TASK_NAME_LENGTH - 1; i++)
    {
        if (source[i] == '\0')
        {
            break;
        }

        destination[i] = source[i];
    }

    destination[i] = '\0';
}

void task_initialize(void)
{
    uint32_t i;

    task_manager.current_task = 0;
    task_manager.next_task_id = 0;
    task_manager.task_count = 0;

    for (i = 0; i < ATLAS_MAX_TASKS; i++)
    {
        task_manager.tasks[i].id = 0;

        task_manager.tasks[i].name[0] = '\0';

        task_manager.tasks[i].state = TASK_TERMINATED;

        task_manager.tasks[i].priority = 0;

        task_manager.tasks[i].stack_base = 0;
        task_manager.tasks[i].stack_pointer = 0;
        task_manager.tasks[i].stack_size = 0;

        task_manager.tasks[i].entry = 0;

        task_manager.tasks[i].context.eax = 0;
        task_manager.tasks[i].context.ebx = 0;
        task_manager.tasks[i].context.ecx = 0;
        task_manager.tasks[i].context.edx = 0;

        task_manager.tasks[i].context.esi = 0;
        task_manager.tasks[i].context.edi = 0;
        task_manager.tasks[i].context.ebp = 0;

        task_manager.tasks[i].context.esp = 0;
        task_manager.tasks[i].context.eip = 0;
        task_manager.tasks[i].context.eflags = 0;
    }
}

AtlasTask* task_get(TaskID id)
{
    uint32_t i;

    for (i = 0; i < ATLAS_MAX_TASKS; i++)
    {
        if (task_manager.tasks[i].id == id &&
            task_manager.tasks[i].state != TASK_TERMINATED)
        {
            return &task_manager.tasks[i];
        }
    }

    return 0;
}

AtlasTask* task_get_current(void)
{
    return task_get(task_manager.current_task);
}

AtlasTask* task_create(const char* name, TaskEntry entry, uint32_t priority)
{
    uint32_t i;
    uint32_t stack_top;
    AtlasTask* task;

    for (i = 0; i < ATLAS_MAX_TASKS; i++)
    {
        if (task_manager.tasks[i].state == TASK_TERMINATED)
        {
            task = &task_manager.tasks[i];

            /*
             * Assign a unique task ID.
             */
            task->id = task_manager.next_task_id;
            task_manager.next_task_id++;

            /*
             * Initialize basic task information.
             */
            task->state = TASK_READY;
            task->priority = priority;
            task->entry = entry;

            task_copy_name(task->name, name);

            /*
             * Give the task its own kernel stack.
             */
            task->stack_base = (uint32_t)&task_stacks[i][0];
            task->stack_size = ATLAS_TASK_STACK_SIZE;

            /*
             * The x86 stack grows downward.
             * Start at the top of the allocated stack.
             */
            stack_top = task->stack_base + task->stack_size;

            stack_top &= 0xFFFFFFF0;

            task->stack_pointer = stack_top - 16;

            /*
             * Initialize the CPU context.
             */
            task->context.eax = 0;
            task->context.ebx = 0;
            task->context.ecx = 0;
            task->context.edx = 0;

            task->context.esi = 0;
            task->context.edi = 0;
            task->context.ebp = stack_top;

            task->context.esp = task->stack_pointer;
            task->context.eip = (uint32_t)entry;

            /*
             * Enable interrupts in the initial EFLAGS.
             */
            task->context.eflags = 0x202;

            task_manager.task_count++;

            return task;
        }
    }

    /*
     * No task slots are available.
     */
    return 0;
}

void task_set_current(TaskID id)
{
    AtlasTask* old_task;
    AtlasTask* new_task;

    new_task = task_get(id);

    if (new_task == 0)
    {
        return;
    }

    old_task = task_get(task_manager.current_task);

    if (old_task != 0 && old_task->id != id)
    {
        old_task->state = TASK_READY;
    }

    task_manager.current_task = id;
    new_task->state = TASK_RUNNING;
}

void task_set_ready(TaskID id)
{
    AtlasTask* task;

    task = task_get(id);

    if (task == 0)
    {
        return;
    }

    task->state = TASK_READY;
}

void task_set_running(TaskID id)
{
    AtlasTask* task;

    task = task_get(id);

    if (task == 0)
    {
        return;
    }

    task->state = TASK_RUNNING;
}

void task_set_blocked(TaskID id)
{
    AtlasTask* task;

    task = task_get(id);

    if (task == 0)
    {
        return;
    }

    task->state = TASK_BLOCKED;
}

void task_set_terminated(TaskID id)
{
    AtlasTask* task;

    task = task_get(id);

    if (task == 0)
    {
        return;
    }

    task->state = TASK_TERMINATED;
}

void task_cleanup(TaskID id)
{
    uint32_t i;
    AtlasTask* task;

    task = 0;

    for (i = 0; i < ATLAS_MAX_TASKS; i++)
    {
        if (task_manager.tasks[i].id == id)
        {
            task = &task_manager.tasks[i];
            break;
        }
    }

    if (task == 0)
    {
        return;
    }

    if (task->state != TASK_TERMINATED)
    {
        return;
    }

    task->name[0] = '\0';
    task->priority = 0;
    task->stack_base = 0;
    task->stack_pointer = 0;
    task->stack_size = 0;
    task->entry = 0;

    task->context.eax = 0;
    task->context.ebx = 0;
    task->context.ecx = 0;
    task->context.edx = 0;
    task->context.esi = 0;
    task->context.edi = 0;
    task->context.ebp = 0;
    task->context.esp = 0;
    task->context.eip = 0;
    task->context.eflags = 0;

    task->id = 0;

    if (task_manager.task_count > 0)
    {
        task_manager.task_count--;
    }
}

void task_terminate(TaskID id)
{
    AtlasTask* task;

    task = task_get(id);

    if (task == 0)
    {
        return;
    }

    if (id == task_manager.current_task)
    {
        return;
    }

    task_set_terminated(id);
    task_cleanup(id);
}

uint32_t task_get_count(void)
{
    return task_manager.task_count;
}

AtlasTask* task_get_by_index(uint32_t index)
{
    if (index >= ATLAS_MAX_TASKS)
    {
        return 0;
    }

    return &task_manager.tasks[index];
}