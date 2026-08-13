#include <stdint.h>

#include "kernel.h"
#include "terminal.h"
#include "keyboard.h"
#include "shell.h"
#include "system.h"
#include "cpu.h"
#include "memory.h"
#include "timer.h"
#include "interrupts.h"
#include "disk.h"
#include "filesystem.h"

void kernel_main(uint32_t multiboot_information)
{
    terminal_initialize();

    system_initialize();

    cpu_initialize();

    memory_initialize(multiboot_information);

    timer_initialize();

    interrupts_initialize();

    disk_initialize();

    filesystem_initialize();

    int filesystem_result;

    filesystem_result =
        filesystem_mount();

    if (filesystem_result ==
        ATLASFS_SUCCESS)
    {
        terminal_write("AtlasFS mounted.\n");
    }
    else if (
        filesystem_result ==
        ATLASFS_INVALID_FS)
    {
        terminal_write(
            "AtlasFS: no valid filesystem.\n"
        );
    }
    else
    {
        terminal_write(
            "AtlasFS: mount failed.\n"
        );
    }

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