#include "kernel.h"
#include "terminal.h"
#include "keyboard.h"

void kernel_main(void)
{
    terminal_initialize();

    terminal_write("NEPTUNE OS ATLAS\n");
    terminal_write("Build 003\n");
    terminal_write("\n");
    terminal_write("Kernel initialized successfully.\n");
    terminal_write("Welcome to Neptune OS Atlas.\n");

    keyboard_initialize();

    terminal_write("> ");

    while (1)
    {
        uint16_t key = keyboard_getkey();

        if (key < 0x100)
        {
            terminal_putchar((char)key);
        }
        else if (key == KEY_ENTER)
        {
            terminal_putchar('\n');
            terminal_write("> ");
        }
        else if (key == KEY_BACKSPACE)
        {
            terminal_backspace();
        }
        else if (key == KEY_TAB)
        {
            terminal_putchar(' ');
            terminal_putchar(' ');
            terminal_putchar(' ');
            terminal_putchar(' ');
        }
    }
}