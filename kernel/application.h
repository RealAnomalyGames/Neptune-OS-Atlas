#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>

#define APPLICATION_NAME_LENGTH 64
#define APPLICATION_MAX_COUNT 16

typedef enum
{
    APPLICATION_STATE_STOPPED = 0,
    APPLICATION_STATE_RUNNING
} ApplicationState;

typedef struct
{
    uint32_t id;

    char name[APPLICATION_NAME_LENGTH];

    uint32_t window_id;

    ApplicationState state;
} Application;

void application_manager_initialize(void);

uint8_t application_manager_is_initialized(void);

uint32_t application_get_count(void);

int32_t application_create(
    const char* name
);

Application* application_get(uint32_t id);

Application* application_get_by_window(
    uint32_t window_id
);

void application_set_window(
    uint32_t id,
    uint32_t window_id
);

uint32_t application_get_window(
    uint32_t id
);

int32_t application_launch(
    const char* name,
    int32_t x,
    int32_t y,
    uint32_t width,
    uint32_t height,
    const char* title
);

void application_close(
    uint32_t id
);

void application_set_state(
    uint32_t id,
    ApplicationState state
);

ApplicationState application_get_state(
    uint32_t id
);

uint8_t application_is_running(
    uint32_t id
);

void application_render(uint32_t id);

#endif