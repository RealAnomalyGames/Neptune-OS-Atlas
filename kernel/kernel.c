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
        char character = keyboard_getchar();

        terminal_putchar(character);
    }
}