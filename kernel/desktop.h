#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>

void desktop_initialize(void);
void desktop_render(void);

uint8_t desktop_is_initialized(void);

void desktop_handle_mouse_click(
    int32_t x,
    int32_t y
);

#endif