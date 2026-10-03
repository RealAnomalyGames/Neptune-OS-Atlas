#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define ATLAS_MAX_TASKS 32
#define ATLAS_TASK_NAME_LENGTH 32
#define ATLAS_TASK_STACK_SIZE 4096

typedef uint32_t TaskID;

typedef enum
{
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED

} TaskState;

typedef void (*TaskEntry)(void);

typedef struct
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;

    uint32_t esp;
    uint32_t eip;

    uint32_t eflags;

} TaskContext;

typedef struct
{
    TaskID id;

    char name[ATLAS_TASK_NAME_LENGTH];

    TaskState state;

    uint32_t priority;

    uint32_t stack_base;
    uint32_t stack_pointer;
    uint32_t stack_size;

    TaskEntry entry;

    TaskContext context;

} AtlasTask;

typedef struct
{
    AtlasTask tasks[ATLAS_MAX_TASKS];

    TaskID current_task;
    TaskID next_task_id;

    uint32_t task_count;

} AtlasTaskManager;

void task_initialize(void);

AtlasTask* task_get(TaskID id);

AtlasTask* task_get_current(void);

AtlasTask* task_create(const char* name, TaskEntry entry, uint32_t priority);

void task_set_current(TaskID id);

void task_set_ready(TaskID id);

void task_set_running(TaskID id);

void task_set_blocked(TaskID id);

void task_set_terminated(TaskID id);

void task_terminate(TaskID id);

void task_cleanup(TaskID id);

uint32_t task_get_count(void);

AtlasTask* task_get_by_index(uint32_t index);

void task_context_switch(AtlasTask* current, AtlasTask* next);

#endif