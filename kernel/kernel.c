#include "kernel.h"
#include "terminal.h"

void kernel_main(void)
{
    terminal_initialize();

    terminal_write("NEPTUNE OS ATLAS\n");
    terminal_write("Build 002\n");
    terminal_write("\n");
    terminal_write("Kernel initialized successfully.\n");
    terminal_write("Welcome to Neptune OS Atlas.\n");

    while (1)
    {
        __asm__ volatile ("hlt");
    }
}