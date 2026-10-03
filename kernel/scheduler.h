#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"

void scheduler_initialize(void);

TaskID scheduler_get_next_task(void);

void scheduler_schedule(void);

#endif