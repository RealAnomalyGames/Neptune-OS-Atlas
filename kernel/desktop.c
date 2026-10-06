#include "desktop.h"
#include "graphics.h"
#include "window.h"
#include "taskbar.h"

#define DESKTOP_BACKGROUND_COLOR ATLAS_COLOR_DARK_BLUE
#define DESKTOP_BAR_COLOR        ATLAS_COLOR_BLUE
#define DESKTOP_TEXT_COLOR       ATLAS_COLOR_WHITE
#define DESKTOP_ACCENT_COLOR     ATLAS_COLOR_CYAN

static uint8_t desktop_initialized;

void desktop_initialize(void)
{
    taskbar_initialize();

    desktop_initialized = 1;
}

uint8_t desktop_is_initialized(void)
{
    return desktop_initialized;
}

void desktop_render(void)
{
    if (desktop_initialized == 0)
    {
        return;
    }

    graphics_clear(
        DESKTOP_BACKGROUND_COLOR
    );

    /*
     * Desktop top bar.
     */
    graphics_fill_rect(
        0,
        0,
        GRAPHICS_WIDTH,
        18,
        DESKTOP_BAR_COLOR
    );

    /*
     * Atlas branding.
     */
    graphics_draw_text(
        8,
        5,
        "ATLAS",
        DESKTOP_TEXT_COLOR
    );

    /*
     * Operating system name.
     */
    graphics_draw_text(
        16,
        32,
        "NEPTUNE OS ATLAS",
        DESKTOP_TEXT_COLOR
    );

    graphics_draw_text(
        16,
        44,
        "DESKTOP",
        DESKTOP_ACCENT_COLOR
    );

    /*
     * Build identifier.
     */
    graphics_draw_text(
        272,
        5,
        "013",
        DESKTOP_ACCENT_COLOR
    );

    taskbar_render();
}

void desktop_handle_mouse_click(int32_t x, int32_t y)
{
    int32_t window_id;

    if (desktop_initialized == 0)
    {
        return;
    }

    if (
        y >= TASKBAR_Y &&
        y < TASKBAR_Y + TASKBAR_HEIGHT
    )
    {
        taskbar_handle_mouse_click(x, y);
        return;
    }

    if (
        taskbar_is_start_menu_open() != 0 &&
        x >= 4 &&
        x < 154 &&
        y >= TASKBAR_Y - 78 &&
        y < TASKBAR_Y
    )
    {
        taskbar_handle_mouse_click(x, y);
        return;
    }

    window_id = window_get_at_position(x, y);

    if (window_id >= 0)
    {
        return;
    }

    window_set_active(0);
}