#ifndef TASKBAR_H
#define TASKBAR_H

#include <stdint.h>
#include "graphics.h"

#define TASKBAR_HEIGHT 18
#define TASKBAR_Y (GRAPHICS_HEIGHT - TASKBAR_HEIGHT)

void taskbar_initialize(void);
void taskbar_render(void);

uint8_t taskbar_is_initialized(void);

#endif