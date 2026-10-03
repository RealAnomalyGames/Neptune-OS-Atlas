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

void terminal_write_at(
    const char* string,
    uint32_t row,
    uint32_t column
)
{
    uint32_t i = 0;
    uint32_t current_column = column;

    while (string[i] != '\0')
    {
        if (current_column >= TERMINAL_WIDTH)
        {
            break;
        }

        terminal_put_entry_at(
            string[i],
            TERMINAL_DEFAULT_COLOR,
            row,
            current_column
        );

        current_column++;
        i++;
    }
}

void terminal_write_uint(uint32_t value)
{
    char buffer[11];
    uint32_t index = 0;

    if (value == 0)
    {
        terminal_putchar('0');
        return;
    }

    while (value > 0)
    {
        buffer[index] =
            '0' + (value % 10);

        value /= 10;
        index++;
    }

    while (index > 0)
    {
        index--;

        terminal_putchar(
            buffer[index]
        );
    }
}

void terminal_write_int(int32_t value)
{
    if (value < 0)
    {
        terminal_putchar('-');

        value = -value;
    }

    terminal_write_uint((uint32_t)value);
}

void terminal_write_uint_at(
    uint32_t value,
    uint32_t row,
    uint32_t column
)
{
    char buffer[11];
    uint32_t index = 0;
    uint32_t current_column = column;

    if (value == 0)
    {
        terminal_put_entry_at(
            '0',
            TERMINAL_DEFAULT_COLOR,
            row,
            current_column
        );

        return;
    }

    while (value > 0)
    {
        buffer[index] =
            '0' + (value % 10);

        value /= 10;
        index++;
    }

    while (index > 0)
    {
        if (current_column >= TERMINAL_WIDTH)
        {
            break;
        }

        index--;

        terminal_put_entry_at(
            buffer[index],
            TERMINAL_DEFAULT_COLOR,
            row,
            current_column
        );

        current_column++;
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