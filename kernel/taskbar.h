#ifndef TASKBAR_H
#define TASKBAR_H

#include <stdint.h>
#include "graphics.h"

#define TASKBAR_HEIGHT 18
#define TASKBAR_Y (GRAPHICS_HEIGHT - TASKBAR_HEIGHT)

void taskbar_initialize(void);
void taskbar_render(void);

uint8_t taskbar_is_initialized(void);

void taskbar_handle_mouse_click(int32_t x, int32_t y);
uint8_t taskbar_is_start_menu_open(void);

#endif