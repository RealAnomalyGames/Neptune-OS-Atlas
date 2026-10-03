#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

/*
 * Logical mouse button states.
 */
#define MOUSE_BUTTON_LEFT   0x01
#define MOUSE_BUTTON_RIGHT  0x02
#define MOUSE_BUTTON_MIDDLE 0x04

/*
 * Logical mouse coordinate limits.
 */
#define MOUSE_MIN_X 0
#define MOUSE_MIN_Y 0
#define MOUSE_MAX_X 639
#define MOUSE_MAX_Y 479

/*
 * Current state of the mouse.
 */
typedef struct
{
    int32_t x;
    int32_t y;

    int32_t delta_x;
    int32_t delta_y;

    uint8_t buttons;

    uint8_t left_button;
    uint8_t right_button;
    uint8_t middle_button;
} MouseState;

typedef struct
{
    int32_t x;
    int32_t y;
    uint8_t visible;
} CursorState;

/*
 * Initialize the mouse driver.
 */
void mouse_initialize(void);

/*
 * Get the current mouse state.
 */
void mouse_get_state(MouseState* state);

/*
 * Get the current X position.
 */
int32_t mouse_get_x(void);

/*
 * Get the current Y position.
 */
int32_t mouse_get_y(void);

/*
 * Get the most recent X movement.
 */
int32_t mouse_get_delta_x(void);

/*
 * Get the most recent Y movement.
 */
int32_t mouse_get_delta_y(void);

/*
 * Get the current button bitmask.
 */
uint8_t mouse_get_buttons(void);

/*
 * Check whether the left button is pressed.
 */
uint8_t mouse_is_left_button_pressed(void);

/*
 * Check whether the right button is pressed.
 */
uint8_t mouse_is_right_button_pressed(void);

/*
 * Check whether the middle button is pressed.
 */
uint8_t mouse_is_middle_button_pressed(void);

void mouse_interrupt_handler(void);

void cursor_initialize(void);
void cursor_update(void);
void cursor_get_state(CursorState* state);

int32_t cursor_get_x(void);
int32_t cursor_get_y(void);

void cursor_set_visible(uint8_t visible);
uint8_t cursor_is_visible(void);

void mouse_clear_delta(void);

void mouse_display_status(void);

#endif