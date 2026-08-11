#include "terminal.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static volatile uint16_t* const terminal_buffer =
    (uint16_t*)0xB8000;

static uint8_t vga_entry_color(
    enum vga_color foreground,
    enum vga_color background
)
{
    return foreground | background << 4;
}

static uint16_t vga_entry(
    unsigned char character,
    uint8_t color
)
{
    return (uint16_t)character | (uint16_t)color << 8;
}

void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;

    terminal_color = vga_entry_color(
        VGA_COLOR_LIGHT_GREY,
        VGA_COLOR_BLACK
    );

    for (size_t y = 0; y < VGA_HEIGHT; y++)
    {
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            const size_t index = y * VGA_WIDTH + x;

            terminal_buffer[index] =
                vga_entry(' ', terminal_color);
        }
    }
}

void terminal_putchar(char c)
{
    if (c == '\n')
    {
        terminal_column = 0;
        terminal_row++;

        if (terminal_row == VGA_HEIGHT)
        {
            terminal_row = 0;
        }

        return;
    }

    const size_t index =
        terminal_row * VGA_WIDTH + terminal_column;

    terminal_buffer[index] =
        vga_entry((unsigned char)c, terminal_color);

    terminal_column++;

    if (terminal_column == VGA_WIDTH)
    {
        terminal_column = 0;
        terminal_row++;

        if (terminal_row == VGA_HEIGHT)
        {
            terminal_row = 0;
        }
    }
}

void terminal_write(const char* data)
{
    while (*data)
    {
        terminal_putchar(*data);
        data++;
    }
}