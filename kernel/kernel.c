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
#include "desktop.h"
#include "graphics.h"
#include "window.h"
#include "taskbar.h"
#include "application.h"
#include "settings.h"

static void kernel_task(void)
{
    while (1)
    {
    }
}

void kernel_main(uint32_t multiboot_information)
{
    uint8_t previous_left_button;
    uint8_t current_left_button;

    previous_left_button = 0;
    current_left_button = 0;

    gdt_initialize();

    graphics_initialize();

    window_manager_initialize();

    desktop_initialize();

    application_manager_initialize();

    settings_manager_initialize();

    interrupts_initialize();

    mouse_initialize();
    cursor_initialize();

    cursor_set_visible(
        settings_get_cursor_visible()
    );

    interrupts_register_handler(
        IRQ12,
        mouse_interrupt_handler
    );

    pic_unmask_irq(12);

    window_manager_redraw();

    interrupts_enable();

    while (1)
    {
        int32_t mouse_x;
        int32_t mouse_y;
        int32_t window_id;

        current_left_button =
            mouse_is_left_button_pressed();

        if (current_left_button != 0)
        {
            terminal_write_at(
                "LEFT DOWN",
                0,
                0
            );
        }
        else
        {
            terminal_write_at(
                "LEFT UP  ",
                0,
                0
            );
        }

        mouse_x = cursor_get_x();
        mouse_y = cursor_get_y();

        cursor_set_visible(
            settings_get_cursor_visible()
        );

        /*
         * Left mouse button was just pressed.
         */
        if (
            current_left_button != 0 &&
            previous_left_button == 0
        )
        {
            window_id = window_get_at_position(
                mouse_x,
                mouse_y
            );

            if (window_id >= 0)
            {
                Window* window;
                uint32_t application_id;

                window = window_get(
                    (uint32_t)window_id
                );

                if (window != 0)
                {
                    /*
                     * Close button requires an actual
                     * left-button press.
                     */
                    if (
                        current_left_button != 0 &&
                        window_is_close_button_at_position(
                            (uint32_t)window_id,
                            mouse_x,
                            mouse_y
                        ) != 0
                    )
                    {
                        application_id =
                            window_get_owner(
                                (uint32_t)window_id
                            );

                        if (application_id != 0)
                        {
                            application_close(
                                application_id
                            );
                        }
                        else
                        {
                            window_destroy(
                                (uint32_t)window_id
                            );
                        }
                    }
                    else if (
                        mouse_y >= window->y &&
                        mouse_y < window->y + 12
                    )
                    {
                        window_begin_drag(
                            (uint32_t)window_id,
                            mouse_x,
                            mouse_y
                        );
                    }
                    else
                    {
                        uint32_t owner_id;
                        Application* application;

                        owner_id =
                            window_get_owner(
                                (uint32_t)window_id
                            );

                        application = 0;

                        if (owner_id != 0)
                        {
                            application =
                                application_get(owner_id);
                        }

                        if (
                            application != 0 &&
                            application->name[0] == 'S' &&
                            application->name[1] == 'e' &&
                            application->name[2] == 't' &&
                            application->name[3] == 't' &&
                            application->name[4] == 'i' &&
                            application->name[5] == 'n' &&
                            application->name[6] == 'g' &&
                            application->name[7] == 's' &&
                            application->name[8] == '\0'
                        )
                        {
            /*
             * Settings controls.
             */
                            settings_handle_click(
                                application->id,
                                mouse_x,
                                mouse_y
                            );

                            window_set_active(
                                (uint32_t)window_id
                            );

                            window_manager_redraw();
                        }
                        else
                        {
                            window_set_active(
                                (uint32_t)window_id
                            );
                        }
                    }
                }
            }
            else
            {
                desktop_handle_mouse_click(
                    mouse_x,
                    mouse_y
                );
            }
        }

        /*
         * Window is currently being dragged.
         */
        if (current_left_button != 0)
        {
            uint32_t active_window;

            active_window = window_get_active();

            if (
                active_window != 0 &&
                window_is_dragging(active_window) != 0
            )
            {
                window_update_drag(
                    active_window,
                    mouse_x,
                    mouse_y
                );
            }
        }

        /*
         * Left mouse button was released.
         */
        if (
            current_left_button == 0 &&
            previous_left_button != 0
        )
        {
            uint32_t active_window;

            active_window = window_get_active();

            if (active_window != 0)
            {
                window_end_drag(active_window);
            }
        }

        /*
         * Redraw the window manager whenever the
         * mouse position or button state changes.
         */
        if (
            mouse_get_delta_x() != 0 ||
            mouse_get_delta_y() != 0 ||
            current_left_button != previous_left_button ||
            window_manager_needs_redraw() != 0
        )
        {
            window_manager_redraw();
            window_manager_clear_redraw();
        }

        previous_left_button = current_left_button;

        mouse_clear_delta();
    }
}