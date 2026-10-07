#include "application.h"
#include "window.h"
#include "settings.h"
#include "files.h"

static Application applications[APPLICATION_MAX_COUNT];
static uint32_t next_application_id;
static uint8_t application_manager_initialized;

void application_manager_initialize(void)
{
    uint32_t i;

    next_application_id = 1;
    application_manager_initialized = 0;

    for (i = 0; i < APPLICATION_MAX_COUNT; i++)
    {
        applications[i].id = 0;
        applications[i].name[0] = '\0';
        applications[i].window_id = 0;
        applications[i].state =
            APPLICATION_STATE_STOPPED;
    }

    application_manager_initialized = 1;
}

uint8_t application_manager_is_initialized(void)
{
    return application_manager_initialized;
}

uint32_t application_get_count(void)
{
    uint32_t i;
    uint32_t count;

    count = 0;

    for (i = 0; i < APPLICATION_MAX_COUNT; i++)
    {
        if (
            applications[i].state ==
            APPLICATION_STATE_RUNNING
        )
        {
            count++;
        }
    }

    return count;
}

int32_t application_create(
    const char* name
)
{
    uint32_t i;
    uint32_t name_index;
    Application* application;

    if (application_manager_initialized == 0)
    {
        return -1;
    }

    if (name == 0)
    {
        return -1;
    }

    for (i = 0; i < APPLICATION_MAX_COUNT; i++)
    {
        if (applications[i].id == 0)
        {
            application = &applications[i];

            application->id = next_application_id;

            next_application_id++;

            if (next_application_id == 0)
            {
                next_application_id = 1;
            }

            application->window_id = 0;
            application->state =
                APPLICATION_STATE_RUNNING;

            for (
                name_index = 0;
                name_index < APPLICATION_NAME_LENGTH - 1;
                name_index++
            )
            {
                application->name[name_index] =
                    name[name_index];

                if (name[name_index] == '\0')
                {
                    break;
                }
            }

            application->name[
                APPLICATION_NAME_LENGTH - 1
            ] = '\0';

            return (int32_t)application->id;
        }
    }

    return -1;
}

Application* application_get(uint32_t id)
{
    uint32_t i;

    if (id == 0)
    {
        return 0;
    }

    for (i = 0; i < APPLICATION_MAX_COUNT; i++)
    {
        if (applications[i].id == id)
        {
            return &applications[i];
        }
    }

    return 0;
}

Application* application_get_by_window(
    uint32_t window_id
)
{
    uint32_t i;

    if (window_id == 0)
    {
        return 0;
    }

    for (i = 0; i < APPLICATION_MAX_COUNT; i++)
    {
        if (
            applications[i].id != 0 &&
            applications[i].window_id == window_id
        )
        {
            return &applications[i];
        }
    }

    return 0;
}

void application_set_window(
    uint32_t id,
    uint32_t window_id
)
{
    Application* application;

    application = application_get(id);

    if (application == 0)
    {
        return;
    }

    application->window_id = window_id;
}

uint32_t application_get_window(
    uint32_t id
)
{
    Application* application;

    application = application_get(id);

    if (application == 0)
    {
        return 0;
    }

    return application->window_id;
}

int32_t application_launch(
    const char* name,
    int32_t x,
    int32_t y,
    uint32_t width,
    uint32_t height,
    const char* title
)
{
    int32_t application_id;
    int32_t window_id;
    Application* application;

    if (name == 0)
    {
        return -1;
    }

    if (title == 0)
    {
        return -1;
    }

    application_id = application_create(name);

    if (application_id < 0)
    {
        return -1;
    }

    window_id = window_create(
        x,
        y,
        width,
        height,
        title
    );

    if (window_id < 0)
    {
        application = application_get(
            (uint32_t)application_id
        );

        if (application != 0)
        {
            application->id = 0;
            application->name[0] = '\0';
            application->window_id = 0;
            application->state =
                APPLICATION_STATE_STOPPED;
        }

        return -1;
    }

    application_set_window(
        (uint32_t)application_id,
        (uint32_t)window_id
    );

    window_set_owner(
        (uint32_t)window_id,
        (uint32_t)application_id
    );

    window_set_active(
        (uint32_t)window_id
    );

    return application_id;
}

void application_close(
    uint32_t id
)
{
    Application* application;
    uint32_t window_id;

    application = application_get(id);

    if (application == 0)
    {
        return;
    }

    /*
     * Give application-specific systems a chance
     * to release their state before the application
     * record is cleared.
     */
    files_shutdown(id);

    window_id = application->window_id;

    if (window_id != 0)
    {
        window_destroy(window_id);
    }

    application->id = 0;
    application->name[0] = '\0';
    application->window_id = 0;
    application->state =
        APPLICATION_STATE_STOPPED;
}

void application_set_state(
    uint32_t id,
    ApplicationState state
)
{
    Application* application;

    application = application_get(id);

    if (application == 0)
    {
        return;
    }

    application->state = state;
}

ApplicationState application_get_state(
    uint32_t id
)
{
    Application* application;

    application = application_get(id);

    if (application == 0)
    {
        return APPLICATION_STATE_STOPPED;
    }

    return application->state;
}

uint8_t application_is_running(
    uint32_t id
)
{
    Application* application;

    application = application_get(id);

    if (application == 0)
    {
        return 0;
    }

    if (
        application->state ==
        APPLICATION_STATE_RUNNING
    )
    {
        return 1;
    }

    return 0;
}

void application_render(uint32_t id)
{
    Application* application;

    application = application_get(id);

    if (application == 0)
    {
        return;
    }

    if (application->state != APPLICATION_STATE_RUNNING)
    {
        return;
    }

    if (
        application->name[0] == 'S' &&
        application->name[1] == 'e' &&
        application->name[2] == 't' &&
        application->name[3] == 't' &&
        application->name[4] == 'i' &&
        application->name[5] == 'n' &&
        application->name[6] == 'g' &&
        application->name[7] == 's' &&
        application->name[8] == '\0'
    )
    {
        settings_render(id);
    }
    if (
        application->name[0] == 'F' &&
        application->name[1] == 'i' &&
        application->name[2] == 'l' &&
        application->name[3] == 'e' &&
        application->name[4] == 's' &&
        application->name[5] == '\0'
    )
    {
        files_render(id);
    }
}