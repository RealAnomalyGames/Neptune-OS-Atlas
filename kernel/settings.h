#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

#define SETTINGS_MOUSE_SPEED_SLOW    0
#define SETTINGS_MOUSE_SPEED_NORMAL  1
#define SETTINGS_MOUSE_SPEED_FAST    2

#define SETTINGS_BACKGROUND_DEFAULT  0

#define SETTINGS_DEFAULT_CURSOR_VISIBLE  1
#define SETTINGS_DEFAULT_BACKGROUND      SETTINGS_BACKGROUND_DEFAULT
#define SETTINGS_DEFAULT_MOUSE_SPEED     SETTINGS_MOUSE_SPEED_NORMAL

typedef struct
{
    uint8_t cursor_visible;
    uint8_t background;
    uint8_t mouse_speed;
} Settings;

void settings_manager_initialize(void);
uint8_t settings_manager_is_initialized(void);

Settings* settings_get(void);

void settings_set_cursor_visible(uint8_t visible);
uint8_t settings_get_cursor_visible(void);

void settings_set_background(uint8_t background);
uint8_t settings_get_background(void);

void settings_set_mouse_speed(uint8_t speed);
uint8_t settings_get_mouse_speed(void);

int32_t settings_launch(void);

void settings_render(uint32_t application_id);

void settings_handle_click(
    uint32_t application_id,
    int32_t x,
    int32_t y
);

#endif