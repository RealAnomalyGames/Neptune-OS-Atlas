#include "terminal.h"

#define VGA_MEMORY ((uint16_t*)0xB8000)

#define TERMINAL_DEFAULT_COLOR 0x07

static uint32_t terminal_row;
static uint32_t terminal_column;

static uint16_t terminal_entry(
    unsigned char character,
    unsigned char color
)
{
    return ((uint16_t)color << 8) | character;
}

static void terminal_put_entry_at(
    char character,
    unsigned char color,
    uint32_t row,
    uint32_t column
)
{
    uint32_t index;

    if (row >= TERMINAL_HEIGHT)
    {
        return;
    }

    if (column >= TERMINAL_WIDTH)
    {
        return;
    }

    index = row * TERMINAL_WIDTH + column;

    VGA_MEMORY[index] = terminal_entry(
        character,
        color
    );
}

void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;

    terminal_clear();
}

void terminal_clear(void)
{
    for (uint32_t row = 0; row < TERMINAL_HEIGHT; row++)
    {
        for (uint32_t column = 0; column < TERMINAL_WIDTH; column++)
        {
            terminal_put_entry_at(
                ' ',
                TERMINAL_DEFAULT_COLOR,
                row,
                column
            );
        }
    }

    terminal_row = 0;
    terminal_column = 0;
}

void terminal_newline(void)
{
    terminal_column = 0;
    terminal_row++;

    if (terminal_row >= TERMINAL_HEIGHT)
    {
        terminal_scroll();
    }
}

void terminal_scroll(void)
{
    for (uint32_t row = 1; row < TERMINAL_HEIGHT; row++)
    {
        for (uint32_t column = 0; column < TERMINAL_WIDTH; column++)
        {
            uint32_t source =
                row * TERMINAL_WIDTH + column;

            uint32_t destination =
                (row - 1) * TERMINAL_WIDTH + column;

            VGA_MEMORY[destination] =
                VGA_MEMORY[source];
        }
    }

    for (uint32_t column = 0; column < TERMINAL_WIDTH; column++)
    {
        terminal_put_entry_at(
            ' ',
            TERMINAL_DEFAULT_COLOR,
            TERMINAL_HEIGHT - 1,
            column
        );
    }

    terminal_row = TERMINAL_HEIGHT - 1;
    terminal_column = 0;
}

void terminal_putchar(char character)
{
    if (character == '\n')
    {
        terminal_newline();
        return;
    }

    if (character == '\r')
    {
        terminal_column = 0;
        return;
    }

    if (character == '\b')
    {
        terminal_backspace();
        return;
    }

    if (terminal_column >= TERMINAL_WIDTH)
    {
        terminal_newline();
    }

    terminal_put_entry_at(
        character,
        TERMINAL_DEFAULT_COLOR,
        terminal_row,
        terminal_column
    );

    terminal_column++;

    if (terminal_column >= TERMINAL_WIDTH)
    {
        terminal_newline();
    }
}

void terminal_write(const char* string)
{
    uint32_t i = 0;

    while (string[i] != '\0')
    {
        terminal_putchar(string[i]);
        i++;
    }
}

void terminal_backspace(void)
{
    if (terminal_column == 0)
    {
        return;
    }

    terminal_column--;

    terminal_put_entry_at(
        ' ',
        TERMINAL_DEFAULT_COLOR,
        terminal_row,
        terminal_column
    );
}

uint32_t terminal_get_row(void)
{
    return terminal_row;
}

uint32_t terminal_get_column(void)
{
    return terminal_column;
}