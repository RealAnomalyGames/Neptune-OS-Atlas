#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

#define WINDOW_TITLE_LENGTH 64
#define WINDOW_MAX_COUNT 16

#define WINDOW_MIN_WIDTH  40
#define WINDOW_MIN_HEIGHT 30

typedef struct
{
    uint32_t id;

    int32_t x;
    int32_t y;

    uint32_t width;
    uint32_t height;

    uint8_t visible;
    uint8_t active;

    char title[WINDOW_TITLE_LENGTH];
} Window;

void window_manager_initialize(void);

int32_t window_create(
    int32_t x,
    int32_t y,
    uint32_t width,
    uint32_t height,
    const char* title
);

Window* window_get(uint32_t id);

void window_destroy(uint32_t id);

void window_set_position(
    uint32_t id,
    int32_t x,
    int32_t y
);

int32_t window_get_x(uint32_t id);
int32_t window_get_y(uint32_t id);

void window_set_size(
    uint32_t id,
    uint32_t width,
    uint32_t height
);

uint32_t window_get_width(uint32_t id);
uint32_t window_get_height(uint32_t id);

void window_render(uint32_t id);
void window_render_all(void);

void window_bring_to_front(uint32_t id);
uint32_t window_get_z_order(uint32_t id);

void window_set_active(uint32_t id);
uint32_t window_get_active(void);
uint8_t window_is_active(uint32_t id);

void window_begin_drag(
    uint32_t id,
    int32_t mouse_x,
    int32_t mouse_y
);

void window_update_drag(
    uint32_t id,
    int32_t mouse_x,
    int32_t mouse_y
);

void window_end_drag(
    uint32_t id
);

uint8_t window_is_dragging(
    uint32_t id
);

int32_t window_get_at_position(
    int32_t x,
    int32_t y
);

void window_manager_redraw(void);

uint8_t window_manager_needs_redraw(void);
void window_manager_clear_redraw(void);

#endif