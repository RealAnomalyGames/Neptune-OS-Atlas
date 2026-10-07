#include "taskbar.h"
#include "window.h"
#include "application.h"
#include "settings.h"
#include "files.h"

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
static uint8_t start_menu_open;

static void taskbar_render_start_menu(void);

void taskbar_initialize(void)
{
    taskbar_initialized = 1;
    start_menu_open = 0;
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
        Application* application;
        uint32_t button_x;
        uint8_t button_color;
        char title[9];
        uint32_t title_index;

        window_id = window_get_z_order_at(i);

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
         * Find the application that owns this window.
         */
        application = application_get_by_window(
            window_id
        );

        /*
         * Use the application name when available.
         * Otherwise, use the window title.
         */
        if (application != 0)
        {
            for (
                title_index = 0;
                title_index < 8;
                title_index++
            )
            {
                title[title_index] =
                    application->name[title_index];

                if (
                    application->name[title_index] ==
                    '\0'
                )
                {
                    break;
                }
            }
        }
        else
        {
            for (
                title_index = 0;
                title_index < 8;
                title_index++
            )
            {
                title[title_index] =
                    window->title[title_index];

                if (
                    window->title[title_index] ==
                    '\0'
                )
                {
                    break;
                }
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
        "014",
        TASKBAR_ACCENT_COLOR
    );

    taskbar_render_start_menu();
}

static void taskbar_render_start_menu(void)
{
    if (start_menu_open == 0)
    {
        return;
    }

    graphics_fill_rect(
        4,
        TASKBAR_Y - 96,
        150,
        94,
        TASKBAR_BUTTON_COLOR
    );

    graphics_draw_rect(
        4,
        TASKBAR_Y - 96,
        150,
        94,
        TASKBAR_BORDER_COLOR
    );

    graphics_draw_text(
        12,
        TASKBAR_Y - 86,
        "ATLAS",
        TASKBAR_TEXT_COLOR
    );

    graphics_draw_text(
        12,
        TASKBAR_Y - 66,
        "Settings",
        TASKBAR_TEXT_COLOR
    );

    graphics_draw_text(
        12,
        TASKBAR_Y - 48,
        "Files",
        TASKBAR_TEXT_COLOR
    );

    graphics_draw_text(
        12,
        TASKBAR_Y - 30,
        "About Atlas",
        TASKBAR_TEXT_COLOR
    );

    graphics_draw_text(
        12,
        TASKBAR_Y - 12,
        "Shutdown",
        TASKBAR_TEXT_COLOR
    );
}

void taskbar_handle_mouse_click(int32_t x, int32_t y)
{
    uint32_t i;
    uint32_t button_count;
    uint32_t window_id;
    Window* window;
    int32_t settings_id;
    int32_t files_id;

    if (taskbar_initialized == 0)
    {
        return;
    }

    /*
     * ATLAS Start button.
     */
    if (
        x >= 4 &&
        x < 52 &&
        y >= TASKBAR_Y + 2 &&
        y < TASKBAR_Y + 16
    )
    {
        start_menu_open =
            start_menu_open == 0;

        window_manager_redraw();
        return;
    }

    /*
     * Settings item in the Start Menu.
     */
    if (start_menu_open != 0)
    {
        if (
            x >= 4 &&
            x < 154 &&
            y >= TASKBAR_Y - 72 &&
            y < TASKBAR_Y - 54
        )
        {
            settings_id = settings_launch();

            if (settings_id >= 0)
            {
                start_menu_open = 0;
            }

            window_manager_redraw();
            return;
        }

                /*
         * Files item in the Start Menu.
         */
        if (
            x >= 4 &&
            x < 154 &&
            y >= TASKBAR_Y - 54 &&
            y < TASKBAR_Y - 36
        )
        {
            files_id = files_launch();

            if (files_id >= 0)
            {
                start_menu_open = 0;
            }

            window_manager_redraw();
            return;
        }

        /*
         * Clicking outside the Start Menu closes it.
         */
        if (
            x < 4 ||
            x >= 154 ||
            y < TASKBAR_Y - 96 ||
            y >= TASKBAR_Y
        )
        {
            start_menu_open = 0;
            window_manager_redraw();
            return;
        }
    }

    /*
     * Application taskbar buttons.
     */
    if (
        y >= TASKBAR_Y + 2 &&
        y < TASKBAR_Y + 16
    )
    {
        button_count = 0;

        for (i = 0; i < WINDOW_MAX_COUNT; i++)
        {
            window_id = window_get_z_order_at(i);

            if (window_id == 0)
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

            if (
                x >=
                    TASKBAR_START_X +
                    button_count *
                    (TASKBAR_BUTTON_WIDTH +
                     TASKBAR_BUTTON_SPACING)
                &&
                x <
                    TASKBAR_START_X +
                    button_count *
                    (TASKBAR_BUTTON_WIDTH +
                     TASKBAR_BUTTON_SPACING) +
                    TASKBAR_BUTTON_WIDTH
            )
            {
                window_bring_to_front(window_id);
                window_set_active(window_id);

                start_menu_open = 0;

                window_manager_redraw();
                return;
            }

            button_count++;
        }
    }
}

uint8_t taskbar_is_start_menu_open(void)
{
    return start_menu_open;
}