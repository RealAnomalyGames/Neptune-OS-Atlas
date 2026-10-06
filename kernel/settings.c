#include "settings.h"
#include "application.h"
#include "window.h"
#include "graphics.h"

static Settings settings;
static uint8_t settings_manager_initialized;

void settings_manager_initialize(void)
{
    settings.cursor_visible =
        SETTINGS_DEFAULT_CURSOR_VISIBLE;

    settings.background =
        SETTINGS_DEFAULT_BACKGROUND;

    settings.mouse_speed =
        SETTINGS_DEFAULT_MOUSE_SPEED;

    settings_manager_initialized = 1;
}

uint8_t settings_manager_is_initialized(void)
{
    return settings_manager_initialized;
}

Settings* settings_get(void)
{
    if (settings_manager_initialized == 0)
    {
        return 0;
    }

    return &settings;
}

void settings_set_cursor_visible(uint8_t visible)
{
    if (settings_manager_initialized == 0)
    {
        return;
    }

    settings.cursor_visible = visible != 0;
}

uint8_t settings_get_cursor_visible(void)
{
    if (settings_manager_initialized == 0)
    {
        return 0;
    }

    return settings.cursor_visible;
}

void settings_set_background(uint8_t background)
{
    if (settings_manager_initialized == 0)
    {
        return;
    }

    settings.background = background;
}

uint8_t settings_get_background(void)
{
    if (settings_manager_initialized == 0)
    {
        return SETTINGS_BACKGROUND_DEFAULT;
    }

    return settings.background;
}

void settings_set_mouse_speed(uint8_t speed)
{
    if (settings_manager_initialized == 0)
    {
        return;
    }

    if (speed > SETTINGS_MOUSE_SPEED_FAST)
    {
        speed = SETTINGS_MOUSE_SPEED_NORMAL;
    }

    settings.mouse_speed = speed;
}

uint8_t settings_get_mouse_speed(void)
{
    if (settings_manager_initialized == 0)
    {
        return SETTINGS_MOUSE_SPEED_NORMAL;
    }

    return settings.mouse_speed;
}

int32_t settings_launch(void)
{
    return application_launch(
        "Settings",
        40,
        35,
        240,
        170,
        "Atlas Settings"
    );
}

void settings_render(uint32_t application_id)
{
    Application* application;
    Window* window;
    const char* mouse_speed_text;
    uint32_t application_count;

    application = application_get(
        (uint32_t)application_id
    );

    if (application == 0)
    {
        return;
    }

    window = window_get(
        application->window_id
    );

    if (window == 0)
    {
        return;
    }

    /*
     * Determine the current mouse speed text.
     */
    if (
        settings_get_mouse_speed() ==
        SETTINGS_MOUSE_SPEED_SLOW
    )
    {
        mouse_speed_text = "SLOW";
    }
    else if (
        settings_get_mouse_speed() ==
        SETTINGS_MOUSE_SPEED_FAST
    )
    {
        mouse_speed_text = "FAST";
    }
    else
    {
        mouse_speed_text = "NORMAL";
    }

    application_count = application_get_count();

    /*
     * Settings heading.
     */
    graphics_draw_text(
        window->x + 8,
        window->y + 24,
        "Settings",
        ATLAS_COLOR_WHITE
    );

    /*
     * Display category.
     */
    graphics_draw_text(
        window->x + 8,
        window->y + 42,
        "Display",
        ATLAS_COLOR_CYAN
    );

    graphics_draw_text(
        window->x + 16,
        window->y + 56,
        "Cursor",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        window->x + 70,
        window->y + 56,
        settings_get_cursor_visible() != 0
            ? "[ON]"
            : "[OFF]",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        window->x + 16,
        window->y + 70,
        "Background",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        window->x + 70,
        window->y + 70,
        "DEFAULT",
        ATLAS_COLOR_WHITE
    );

    /*
     * Input category.
     */
    graphics_draw_text(
        window->x + 8,
        window->y + 88,
        "Input",
        ATLAS_COLOR_CYAN
    );

    graphics_draw_text(
        window->x + 16,
        window->y + 102,
        "Mouse Speed",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        window->x + 88,
        window->y + 102,
        mouse_speed_text,
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        window->x + 150,
        window->y + 102,
        "[CLICK]",
        ATLAS_COLOR_CYAN
    );

    /*
     * System category.
     */
    graphics_draw_text(
        window->x + 8,
        window->y + 118,
        "System",
        ATLAS_COLOR_CYAN
    );

    graphics_draw_text(
        window->x + 16,
        window->y + 132,
        "OS",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        window->x + 70,
        window->y + 132,
        "NEPTUNE ATLAS",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        window->x + 16,
        window->y + 144,
        "Build",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        window->x + 70,
        window->y + 144,
        "013",
        ATLAS_COLOR_CYAN
    );
}

void settings_handle_click(
    uint32_t application_id,
    int32_t x,
    int32_t y
)
{
    Application* application;
    Window* window;
    int32_t relative_x;
    int32_t relative_y;
    uint8_t mouse_speed;

    application = application_get(
        application_id
    );

    if (application == 0)
    {
        return;
    }

    window = window_get(
        application->window_id
    );

    if (window == 0)
    {
        return;
    }

    relative_x = x - window->x;
    relative_y = y - window->y;

    /*
     * Cursor setting.
     *
     * The clickable area is the Cursor row.
     */
    if (
        relative_x >= 8 &&
        relative_x < 220 &&
        relative_y >= 48 &&
        relative_y < 64
    )
    {
        settings_set_cursor_visible(
            settings_get_cursor_visible() == 0
        );

        return;
    }

    /*
     * Mouse speed setting.
     *
     * Each click advances to the next speed.
     */
    if (
        relative_x >= 8 &&
        relative_x < 220 &&
        relative_y >= 94 &&
        relative_y < 110
    )
    {
        mouse_speed =
            settings_get_mouse_speed();

        if (
            mouse_speed ==
            SETTINGS_MOUSE_SPEED_SLOW
        )
        {
            mouse_speed =
                SETTINGS_MOUSE_SPEED_NORMAL;
        }
        else if (
            mouse_speed ==
            SETTINGS_MOUSE_SPEED_NORMAL
        )
        {
            mouse_speed =
                SETTINGS_MOUSE_SPEED_FAST;
        }
        else
        {
            mouse_speed =
                SETTINGS_MOUSE_SPEED_SLOW;
        }

        settings_set_mouse_speed(
            mouse_speed
        );
    }
}