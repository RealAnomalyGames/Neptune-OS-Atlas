#include "shell.h"

static char shell_buffer[SHELL_BUFFER_SIZE];
static uint32_t shell_buffer_length;

void shell_initialize(void)
{
    shell_buffer_length = 0;

    for (uint32_t i = 0; i < SHELL_BUFFER_SIZE; i++)
    {
        shell_buffer[i] = '\0';
    }
}

void shell_add_character(char character)
{
    if (shell_buffer_length >= SHELL_BUFFER_SIZE - 1)
    {
        return;
    }

    shell_buffer[shell_buffer_length] = character;
    shell_buffer_length++;

    shell_buffer[shell_buffer_length] = '\0';
}

void shell_remove_character(void)
{
    if (shell_buffer_length == 0)
    {
        return;
    }

    shell_buffer_length--;

    shell_buffer[shell_buffer_length] = '\0';
}

const char* shell_get_buffer(void)
{
    return shell_buffer;
}

uint32_t shell_get_buffer_length(void)
{
    return shell_buffer_length;
}

void shell_clear_buffer(void)
{
    shell_buffer_length = 0;
    shell_buffer[0] = '\0';
}