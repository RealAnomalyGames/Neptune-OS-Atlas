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
#include "task.h"
#include "scheduler.h"
#include "mouse.h"
#include "pic.h"
#include "gdt.h"
#include "graphics.h"

static void kernel_task(void)
{
    while (1)
    {
    }
}

static void atlas_draw_screen(void)
{
    graphics_clear(
        ATLAS_COLOR_DARK_BLUE
    );

    graphics_fill_rect(
        0,
        0,
        GRAPHICS_WIDTH,
        18,
        ATLAS_COLOR_BLUE
    );

    graphics_draw_text(
        8,
        5,
        "ATLAS",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        280,
        5,
        "009",
        ATLAS_COLOR_CYAN
    );

    graphics_draw_text(
        16,
        28,
        "NEPTUNE OS ATLAS",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        16,
        40,
        "GRAPHICAL SYSTEM",
        ATLAS_COLOR_CYAN
    );

    graphics_draw_rect(
        12,
        58,
        296,
        82,
        ATLAS_COLOR_CYAN
    );

    graphics_draw_text(
        20,
        66,
        "SYSTEM STATUS",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        20,
        82,
        "GRAPHICS       ONLINE",
        ATLAS_COLOR_CYAN
    );

    graphics_draw_text(
        20,
        94,
        "FRAMEBUFFER    320 X 200",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        20,
        106,
        "DISPLAY        VGA MODE 13H",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        20,
        118,
        "FONT           8 X 8 ASCII",
        ATLAS_COLOR_WHITE
    );

    graphics_fill_rect(
        20,
        128,
        6,
        6,
        ATLAS_COLOR_GREEN
    );

    graphics_draw_text(
        32,
        128,
        "SYSTEM READY",
        ATLAS_COLOR_GREEN
    );

    graphics_fill_rect(
        0,
        182,
        GRAPHICS_WIDTH,
        18,
        ATLAS_COLOR_BLUE
    );

    graphics_draw_text(
        8,
        187,
        "NEPTUNE CORPORATION",
        ATLAS_COLOR_WHITE
    );

    graphics_draw_text(
        256,
        187,
        "ATLAS 009",
        ATLAS_COLOR_CYAN
    );
}

static void atlas_draw_cursor(void)
{
    graphics_draw_cursor(
        cursor_get_x(),
        cursor_get_y()
    );
}

void kernel_main(uint32_t multiboot_information)
{
    gdt_initialize();

    graphics_initialize();

    atlas_draw_screen();

    interrupts_initialize();

    mouse_initialize();
    cursor_initialize();

    interrupts_register_handler(
        IRQ12,
        mouse_interrupt_handler
    );

    pic_unmask_irq(12);

    graphics_draw_cursor(
        cursor_get_x(),
        cursor_get_y()
    );

    interrupts_enable();

    int32_t previous_cursor_x;
    int32_t previous_cursor_y;

    previous_cursor_x = cursor_get_x();
    previous_cursor_y = cursor_get_y();

    while (1)
    {
        int32_t current_cursor_x;
        int32_t current_cursor_y;

        current_cursor_x = cursor_get_x();
        current_cursor_y = cursor_get_y();

        if (
            current_cursor_x != previous_cursor_x ||
            current_cursor_y != previous_cursor_y
        )
        {
            atlas_draw_screen();

            graphics_draw_cursor(
                current_cursor_x,
                current_cursor_y
            );

            previous_cursor_x = current_cursor_x;
            previous_cursor_y = current_cursor_y;
        }

        __asm__ volatile ("hlt");
    }
}