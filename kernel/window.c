#include "window.h"
#include "graphics.h"
#include "mouse.h"
#include "desktop.h"

#define WINDOW_BORDER_COLOR       ATLAS_COLOR_CYAN
#define WINDOW_TITLE_BAR_COLOR    ATLAS_COLOR_BLUE
#define WINDOW_BACKGROUND_COLOR   ATLAS_COLOR_DARK_GRAY
#define WINDOW_TITLE_COLOR        ATLAS_COLOR_WHITE
#define WINDOW_INACTIVE_TITLE_BAR_COLOR ATLAS_COLOR_DARK_BLUE

static Window windows[WINDOW_MAX_COUNT];
static uint32_t window_z_order[WINDOW_MAX_COUNT];
static uint32_t next_window_id;
static uint32_t active_window_id;
static uint32_t dragging_window_id;
static int32_t drag_offset_x;
static int32_t drag_offset_y;
static uint8_t window_redraw_needed;

void window_manager_initialize(void)
{
    uint32_t i;

    next_window_id = 1;
    active_window_id = 0;

    dragging_window_id = 0;

    drag_offset_x = 0;
    drag_offset_y = 0;

    window_redraw_needed = 1;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        window_z_order[i] = 0;

        windows[i].id = 0;
        windows[i].x = 0;
        windows[i].y = 0;
        windows[i].width = 0;
        windows[i].height = 0;
        windows[i].visible = 0;
        windows[i].active = 0;
        windows[i].title[0] = '\0';
    }
}

int32_t window_create(
    int32_t x,
    int32_t y,
    uint32_t width,
    uint32_t height,
    const char* title
)
{
    uint32_t i;
    uint32_t j;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        if (windows[i].id == 0)
        {
            windows[i].id = next_window_id;

            next_window_id++;

            windows[i].x = x;
            windows[i].y = y;
            windows[i].width = width;
            windows[i].height = height;

            if (windows[i].width < WINDOW_MIN_WIDTH)
            {
                windows[i].width = WINDOW_MIN_WIDTH;
            }

            if (windows[i].height < WINDOW_MIN_HEIGHT)
            {
                windows[i].height = WINDOW_MIN_HEIGHT;
            }

            if (windows[i].width > GRAPHICS_WIDTH)
            {
                windows[i].width = GRAPHICS_WIDTH;
            }

            if (windows[i].height > GRAPHICS_HEIGHT)
            {
                windows[i].height = GRAPHICS_HEIGHT;
            }

            windows[i].visible = 1;
            windows[i].active = 0;

            for (
                j = 0;
                j < WINDOW_TITLE_LENGTH - 1;
                j++
            )
            {
                if (title == 0 || title[j] == '\0')
                {
                    break;
                }

                windows[i].title[j] = title[j];
            }

            windows[i].title[j] = '\0';

            for (j = 0; j < WINDOW_MAX_COUNT; j++)
            {
                if (window_z_order[j] == 0)
                {
                    window_z_order[j] = windows[i].id;
                    break;
                }
            }

            window_redraw_needed = 1;

            return (int32_t)windows[i].id;
        }
    }

    return -1;
}

Window* window_get(uint32_t id)
{
    uint32_t i;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        if (windows[i].id == id)
        {
            return &windows[i];
        }
    }

    return 0;
}

void window_destroy(uint32_t id)
{
    uint32_t i;
    uint32_t j;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        if (windows[i].id == id)
        {
            if (active_window_id == id)
            {
                active_window_id = 0;
            }

            windows[i].id = 0;
            windows[i].x = 0;
            windows[i].y = 0;
            windows[i].width = 0;
            windows[i].height = 0;
            windows[i].visible = 0;
            windows[i].active = 0;
            windows[i].title[0] = '\0';

            break;
        }
    }

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        if (window_z_order[i] == id)
        {
            for (j = i; j < WINDOW_MAX_COUNT - 1; j++)
            {
                window_z_order[j] = window_z_order[j + 1];
            }

            window_z_order[WINDOW_MAX_COUNT - 1] = 0;

            break;
        }
    }

    window_redraw_needed = 1;
}

void window_set_position(
    uint32_t id,
    int32_t x,
    int32_t y
)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return;
    }

    if (x < 0)
    {
        x = 0;
    }

    if (y < 0)
    {
        y = 0;
    }

    if (window->width >= GRAPHICS_WIDTH)
    {
        x = 0;
    }
    else if (
        x + (int32_t)window->width >
        GRAPHICS_WIDTH
    )
    {
        x = GRAPHICS_WIDTH - window->width;
    }

    if (window->height >= GRAPHICS_HEIGHT)
    {
        y = 0;
    }
    else if (
        y + (int32_t)window->height >
        GRAPHICS_HEIGHT
    )
    {
        y = GRAPHICS_HEIGHT - window->height;
    }

    window->x = x;
    window->y = y;

    window_redraw_needed = 1;
}

int32_t window_get_x(uint32_t id)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return -1;
    }

    return window->x;
}

int32_t window_get_y(uint32_t id)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return -1;
    }

    return window->y;
}

void window_set_size(
    uint32_t id,
    uint32_t width,
    uint32_t height
)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return;
    }

    if (width < WINDOW_MIN_WIDTH)
    {
        width = WINDOW_MIN_WIDTH;
    }

    if (height < WINDOW_MIN_HEIGHT)
    {
        height = WINDOW_MIN_HEIGHT;
    }

    if (width > GRAPHICS_WIDTH)
    {
        width = GRAPHICS_WIDTH;
    }

    if (height > GRAPHICS_HEIGHT)
    {
        height = GRAPHICS_HEIGHT;
    }

    window->width = width;
    window->height = height;

    window_set_position(
        id,
        window->x,
        window->y
    );
}

uint32_t window_get_width(uint32_t id)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return 0;
    }

    return window->width;
}

uint32_t window_get_height(uint32_t id)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return 0;
    }

    return window->height;
}

void window_render(uint32_t id)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return;
    }

    if (window->visible == 0)
    {
        return;
    }

    /*
     * Window background.
     */
    if (window->active != 0)
    {
        graphics_fill_rect(
            (uint16_t)window->x + 1,
            (uint16_t)window->y + 1,
            (uint16_t)window->width - 2,
            10,
            WINDOW_TITLE_BAR_COLOR
        );
    }
    else
    {
        graphics_fill_rect(
            (uint16_t)window->x + 1,
            (uint16_t)window->y + 1,
            (uint16_t)window->width - 2,
            10,
            WINDOW_INACTIVE_TITLE_BAR_COLOR
        );
    }

    /*
     * Window border.
     */
    graphics_draw_rect(
        (uint16_t)window->x,
        (uint16_t)window->y,
        (uint16_t)window->width,
        (uint16_t)window->height,
        WINDOW_BORDER_COLOR
    );

    /*
     * Title bar.
     */
    if (window->height >= 12)
    {
        graphics_fill_rect(
            (uint16_t)window->x + 1,
            (uint16_t)window->y + 1,
            (uint16_t)window->width - 2,
            10,
            WINDOW_TITLE_BAR_COLOR
        );

        graphics_draw_text(
            (uint16_t)window->x + 5,
            (uint16_t)window->y + 2,
            window->title,
            WINDOW_TITLE_COLOR
        );

        graphics_draw_text(
            window->x + 8,
            window->y + 24,
            "Atlas Window",
            WINDOW_TITLE_COLOR
        );

        graphics_draw_text(
            window->x + 8,
            window->y + 36,
            "Build 011",
            ATLAS_COLOR_CYAN
        );

        graphics_draw_text(
            window->x + 8,
            window->y + 52,
            "Drag this window",
            ATLAS_COLOR_WHITE
        );
    }
}

void window_render_all(void)
{
    uint32_t i;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        if (window_z_order[i] != 0)
        {
            window_render(window_z_order[i]);
        }
    }
}

void window_bring_to_front(uint32_t id)
{
    uint32_t i;
    uint32_t position;
    uint32_t last_position;
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return;
    }

    position = WINDOW_MAX_COUNT;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        if (window_z_order[i] == id)
        {
            position = i;
            break;
        }
    }

    if (position == WINDOW_MAX_COUNT)
    {
        return;
    }

    last_position = 0;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        if (window_z_order[i] != 0)
        {
            last_position = i;
        }
    }

    if (position == last_position)
    {
        window_set_active(id);
        return;
    }

    for (i = position; i < last_position; i++)
    {
        window_z_order[i] =
            window_z_order[i + 1];
    }

    window_z_order[last_position] = id;

    window_set_active(id);

    window_redraw_needed = 1;
}

uint32_t window_get_z_order(uint32_t id)
{
    uint32_t i;

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        if (window_z_order[i] == id)
        {
            return i;
        }
    }

    return WINDOW_MAX_COUNT;
}

void window_set_active(uint32_t id)
{
    uint32_t i;
    Window* window;

    if (id == 0)
    {
        for (i = 0; i < WINDOW_MAX_COUNT; i++)
        {
            windows[i].active = 0;
        }

        active_window_id = 0;

        return;
    }

    window = window_get(id);

    if (window == 0)
    {
        return;
    }

    for (i = 0; i < WINDOW_MAX_COUNT; i++)
    {
        windows[i].active = 0;
    }

    window->active = 1;
    active_window_id = id;

    window_redraw_needed = 1;
}

uint32_t window_get_active(void)
{
    return active_window_id;
}

uint8_t window_is_active(uint32_t id)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return 0;
    }

    return window->active;
}

int32_t window_get_at_position(
    int32_t x,
    int32_t y
)
{
    int32_t i;
    uint32_t id;
    Window* window;

    /*
     * Start with the front-most window.
     */
    for (
        i = WINDOW_MAX_COUNT - 1;
        i >= 0;
        i--
    )
    {
        id = window_z_order[i];

        if (id == 0)
        {
            continue;
        }

        window = window_get(id);

        if (window == 0)
        {
            continue;
        }

        if (window->visible == 0)
        {
            continue;
        }

        if (x < window->x)
        {
            continue;
        }

        if (y < window->y)
        {
            continue;
        }

        if (
            x >=
            window->x + (int32_t)window->width
        )
        {
            continue;
        }

        if (
            y >=
            window->y + (int32_t)window->height
        )
        {
            continue;
        }

        return (int32_t)id;
    }

    return -1;
}

void window_begin_drag(
    uint32_t id,
    int32_t mouse_x,
    int32_t mouse_y
)
{
    Window* window;

    window = window_get(id);

    if (window == 0)
    {
        return;
    }

    if (window->visible == 0)
    {
        return;
    }

    window_set_active(id);
    window_bring_to_front(id);

    drag_offset_x = mouse_x - window->x;
    drag_offset_y = mouse_y - window->y;

    dragging_window_id = id;
}

void window_update_drag(
    uint32_t id,
    int32_t mouse_x,
    int32_t mouse_y
)
{
    Window* window;
    int32_t new_x;
    int32_t new_y;

    if (dragging_window_id != id)
    {
        return;
    }

    window = window_get(id);

    if (window == 0)
    {
        return;
    }

    new_x = mouse_x - drag_offset_x;
    new_y = mouse_y - drag_offset_y;

    window_set_position(
        id,
        new_x,
        new_y
    );
}

void window_end_drag(
    uint32_t id
)
{
    if (dragging_window_id == id)
    {
        dragging_window_id = 0;

        drag_offset_x = 0;
        drag_offset_y = 0;
    }
}

uint8_t window_is_dragging(
    uint32_t id
)
{
    return dragging_window_id == id;
}

void window_manager_redraw(void)
{
    /*
     * Draw the Desktop first.
     */
    desktop_render();

    /*
     * Draw all windows from back to front.
     */
    window_render_all();

    /*
     * Draw the cursor last.
     */
    graphics_draw_cursor(
        cursor_get_x(),
        cursor_get_y()
    );

    /*
     * Present the completed frame.
     */
    graphics_present();
}

uint8_t window_manager_needs_redraw(void)
{
    return window_redraw_needed;
}

void window_manager_clear_redraw(void)
{
    window_redraw_needed = 0;
}