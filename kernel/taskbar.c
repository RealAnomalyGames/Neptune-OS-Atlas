#include "taskbar.h"
#include "window.h"

#define TASKBAR_BACKGROUND_COLOR ATLAS_COLOR_BLUE
#define TASKBAR_BUTTON_COLOR     ATLAS_COLOR_DARK_BLUE
#define TASKBAR_ACTIVE_COLOR    ATLAS_COLOR_CYAN
#define TASKBAR_BORDER_COLOR     ATLAS_COLOR_CYAN
#define TASKBAR_TEXT_COLOR       ATLAS_COLOR_WHITE
#define TASKBAR_ACCENT_COLOR     ATLAS_COLOR_CYAN

#define TASKBAR_START_X          60
#define TASKBAR_BUTTON_WIDTH     64
#define TASKBAR_BUTTON_HEIGHT    14
#define TASKBAR_BUTTON_SPACING   4
#define TASKBAR_MAX_BUTTONS      3

static uint8_t taskbar_initialized;

void taskbar_initialize(void)
{
    taskbar_initialized = 1;
}

uint8_t taskbar_is_initialized(void)
{
    return taskbar_initialized;
}

void taskbar_render(void)
{
    uint32_t i;
    uint32_t button_count;

    if (taskbar_initialized == 0)
    {
        return;
    }

    /*
     * Draw the taskbar background.
     */
    graphics_fill_rect(
        0,
        TASKBAR_Y,
        GRAPHICS_WIDTH,
        TASKBAR_HEIGHT,
        TASKBAR_BACKGROUND_COLOR
    );

    /*
     * Draw the ATLAS button.
     */
    graphics_fill_rect(
        4,
        TASKBAR_Y + 2,
        48,
        14,
        TASKBAR_BUTTON_COLOR
    );

    graphics_draw_rect(
        4,
        TASKBAR_Y + 2,
        48,
        14,
        TASKBAR_BORDER_COLOR
    );

    graphics_draw_text(
        12,
        TASKBAR_Y + 5,
        "ATLAS",
        TASKBAR_TEXT_COLOR
    );

    /*
     * Draw the separator after the ATLAS button.
     */
    graphics_draw_line(
        56,
        TASKBAR_Y + 2,
        56,
        TASKBAR_Y + 15,
        TASKBAR_ACCENT_COLOR
    );

    /*
     * Draw buttons for open windows.
     */
    button_count = 0;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        uint32_t window_id;
        Window* window;
        uint32_t button_x;
        uint8_t button_color;
        char title[9];
        uint32_t title_index;

        window_id = window_get_z_order(i);

        if (window_id == WINDOW_MAX_COUNT)
        {
            continue;
        }

        window = window_get(window_id);

        if (window == 0)
        {
            continue;
        }

        if (window->visible == 0)
        {
            continue;
        }

        if (button_count >= TASKBAR_MAX_BUTTONS)
        {
            break;
        }

        button_x =
            TASKBAR_START_X +
            button_count *
            (TASKBAR_BUTTON_WIDTH + TASKBAR_BUTTON_SPACING);

        /*
         * Active windows receive a cyan button.
         */
        if (window->active != 0)
        {
            button_color = TASKBAR_ACTIVE_COLOR;
        }
        else
        {
            button_color = TASKBAR_BUTTON_COLOR;
        }

        graphics_fill_rect(
            button_x,
            TASKBAR_Y + 2,
            TASKBAR_BUTTON_WIDTH,
            TASKBAR_BUTTON_HEIGHT,
            button_color
        );

        graphics_draw_rect(
            button_x,
            TASKBAR_Y + 2,
            TASKBAR_BUTTON_WIDTH,
            TASKBAR_BUTTON_HEIGHT,
            TASKBAR_BORDER_COLOR
        );

        /*
         * Copy up to eight characters of the window title.
         */
        for (title_index = 0; title_index < 8; title_index++)
        {
            title[title_index] = window->title[title_index];

            if (window->title[title_index] == '\0')
            {
                break;
            }
        }

        title[8] = '\0';

        graphics_draw_text(
            button_x + 4,
            TASKBAR_Y + 5,
            title,
            TASKBAR_TEXT_COLOR
        );

        button_count++;
    }

    /*
     * Draw the build indicator.
     */
    graphics_draw_text(
        272,
        TASKBAR_Y + 5,
        "011",
        TASKBAR_ACCENT_COLOR
    );
}