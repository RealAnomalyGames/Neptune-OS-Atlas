#include <stdint.h>

#include "kernel.h"
#include "terminal.h"
#include "keyboard.h"
#include "shell.h"
#include "system.h"
#include "cpu.h"
#include "memory.h"
#include "timer.h"

void kernel_main(uint32_t multiboot_information)
{
    terminal_initialize();

    system_initialize();

    cpu_initialize();

    memory_initialize(multiboot_information);

    timer_initialize();

    keyboard_initialize();
    
    shell_initialize();
    shell_run();

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