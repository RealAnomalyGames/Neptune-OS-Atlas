#include "files.h"
#include "application.h"
#include "window.h"
#include "graphics.h"

static FilesState files_state;

static void files_clear_selection(void);
static void files_select_entry(int32_t index);
static void files_go_back(void);
static void files_load_directory(void);
static int files_open_file(int32_t index);
static void files_close_file(void);
static void files_refresh(void);
static void files_reset_state(void);

void files_manager_initialize(void)
{
    files_reset_state();
    files_state.initialized = 1;
}

static void files_update_current_path(void)
{
    const char* filesystem_path;
    uint32_t i;

    filesystem_path = filesystem_get_current_directory();

    if (filesystem_path == 0)
    {
        files_state.current_path[0] = '/';
        files_state.current_path[1] = '\0';
        return;
    }

    for (i = 0; i < ATLASFS_MAX_PATH - 1; i++)
    {
        files_state.current_path[i] = filesystem_path[i];

        if (filesystem_path[i] == '\0')
        {
            break;
        }
    }

    files_state.current_path[ATLASFS_MAX_PATH - 1] = '\0';
}

static void files_load_directory(void)
{
    AtlasDirectoryEntry filesystem_entries[FILES_MAX_ENTRIES];
    uint32_t count;
    uint32_t i;
    int result;

    files_update_current_path();

    files_state.entry_count = 0;
    files_clear_selection();
    files_state.scroll_offset = 0;
    files_state.last_error = ATLASFS_SUCCESS;

    if (!filesystem_is_mounted())
    {
        files_state.last_error = ATLASFS_NOT_MOUNTED;
        return;
    }

    result = filesystem_list_directory_entries(
        filesystem_entries,
        FILES_MAX_ENTRIES,
        &count
    );

    if (result != ATLASFS_SUCCESS)
    {
        files_state.last_error = result;
        return;
    }

    for (i = 0; i < count; i++)
    {
        files_state.entries[i].metadata_block =
            filesystem_entries[i].metadata_block;

        files_state.entries[i].type =
            filesystem_entries[i].type;

        files_state.entries[i].size =
            filesystem_entries[i].size;

        {
            uint32_t character;

            for (character = 0;
                 character < ATLASFS_MAX_FILENAME;
                 character++)
            {
                files_state.entries[i].name[character] =
                    filesystem_entries[i].name[character];

                if (filesystem_entries[i].name[character] == '\0')
                {
                    break;
                }
            }

            files_state.entries[i].name[
                ATLASFS_MAX_FILENAME - 1
            ] = '\0';
        }
    }

    files_state.entry_count = count;
}

static void files_select_entry(int32_t index)
{
    if (index < 0)
    {
        files_state.selected_index = -1;
        return;
    }

    if ((uint32_t)index >= files_state.entry_count)
    {
        files_state.selected_index = -1;
        return;
    }

    files_state.selected_index = index;
    files_state.last_error = ATLASFS_SUCCESS;
}

static void files_clear_selection(void)
{
    files_state.selected_index = -1;
}

static void files_go_back(void)
{
    int result;

    result = filesystem_change_directory("..");

    if (result != ATLASFS_SUCCESS)
    {
        files_state.last_error = result;
        return;
    }

    files_load_directory();
}

static void files_reset_state(void)
{
    uint32_t i;

    files_state.application_id = 0;
    files_state.window_id = 0;

    files_state.current_path[0] = '/';
    files_state.current_path[1] = '\0';

    files_state.entry_count = 0;
    files_state.selected_index = -1;
    files_state.scroll_offset = 0;
    files_state.last_error = ATLASFS_SUCCESS;

    files_state.file_open = 0;
    files_state.opened_file_name[0] = '\0';
    files_state.opened_file_size = 0;

    for (i = 0; i < FILES_MAX_ENTRIES; i++)
    {
        files_state.entries[i].metadata_block = 0;
        files_state.entries[i].name[0] = '\0';
        files_state.entries[i].type = ATLASFS_TYPE_FREE;
        files_state.entries[i].size = 0;
    }
}

int32_t files_launch(void)
{
    int32_t application_id;
    uint32_t window_id;

    if (!files_state.initialized)
    {
        files_manager_initialize();
    }

    /*
     * Reset Files-specific state for a new launch.
     */
    files_reset_state();

    files_state.initialized = 1;

    application_id = application_launch(
        "Files",
        30,
        20,
        260,
        170,
        "Atlas Files"
    );

    if (application_id < 0)
    {
        return application_id;
    }

    window_id = application_get_window(
        (uint32_t)application_id
    );

    files_state.application_id =
        (uint32_t)application_id;

    files_state.window_id =
        window_id;

    files_load_directory();

    return application_id;
}

void files_render(uint32_t application_id)
{
    Application* application;
    Window* window;

    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;

    application = application_get(application_id);

    if (application == 0)
    {
        return;
    }

    window = window_get(application->window_id);

    if (window == 0)
    {
        return;
    }

    x = (uint16_t)window->x;
    y = (uint16_t)window->y;
    width = (uint16_t)window->width;
    height = (uint16_t)window->height;

    /*
     * Files client area.
     *
     * The window manager already draws the title bar,
     * so everything below begins inside the client area.
     */

    /*
     * Path bar.
     */
    graphics_fill_rect(
        x + 6,
        y + 11,
        width - 12,
        16,
        ATLAS_COLOR_WHITE
    );

    graphics_draw_rect(
        x + 6,
        y + 11,
        width - 12,
        16,
        ATLAS_COLOR_DARK_GRAY
    );

    if (files_state.file_open != 0)
    {
        graphics_draw_text(
            x + 10,
            y + 16,
            files_state.opened_file_name,
            ATLAS_COLOR_BLACK
        );
    }
    else
    {
        graphics_draw_text(
            x + 10,
            y + 16,
            files_state.current_path,
            ATLAS_COLOR_BLACK
        );
    }

    /*
     * Back button.
     */
    graphics_fill_rect(
        x + 6,
        y + 29,
        40,
        16,
        ATLAS_COLOR_GRAY
    );

    graphics_draw_rect(
        x + 6,
        y + 29,
        40,
        16,
        ATLAS_COLOR_DARK_GRAY
    );

    graphics_draw_text(
        x + 14,
        y + 34,
        "Back",
        ATLAS_COLOR_BLACK
    );

    /*
     * Refresh button.
     */
    graphics_fill_rect(
        x + width - 56,
        y + 29,
        50,
        16,
        ATLAS_COLOR_GRAY
    );

    graphics_draw_rect(
        x + width - 56,
        y + 29,
        50,
        16,
        ATLAS_COLOR_DARK_GRAY
    );

    graphics_draw_text(
        x + width - 59,
        y + 34,
        "Refresh",
        ATLAS_COLOR_BLACK
    );

    /*
     * File viewer.
     */
    if (files_state.file_open != 0)
    {
        uint32_t i;
        uint32_t line;
        uint32_t column;
        uint32_t display_y;
        uint8_t character;

        graphics_fill_rect(
            x + 6,
            y + 48,
            width - 12,
            height - 72,
            ATLAS_COLOR_WHITE
        );

        graphics_draw_rect(
            x + 6,
            y + 48,
            width - 12,
            height - 72,
            ATLAS_COLOR_DARK_GRAY
        );

        line = 0;
        column = 0;
        display_y = y + 53;

        for (i = 0; i < files_state.opened_file_size; i++)
        {
            character = files_state.opened_file_data[i];

            /*
             * Stop when the visible viewer area is full.
             */
            if (display_y + 8 >= y + height - 21)
            {
                break;
            }

            /*
             * New line.
             */
            if (character == '\n')
            {
                line++;
                column = 0;
                display_y += 10;
                continue;
            }

            /*
             * Ignore carriage returns.
             */
            if (character == '\r')
            {
                continue;
            }

            /*
             * Keep the first version of the viewer
             * limited to printable ASCII characters.
             */
            if (
                character >= 32 &&
                character <= 126
            )
            {
                graphics_draw_char(
                    x + 10 + (uint16_t)(column * 8),
                    display_y,
                    (char)character,
                    ATLAS_COLOR_BLACK
                );

                column++;

                /*
                 * Keep text inside the window.
                 */
                if (column >= 29)
                {
                    column = 0;
                    line++;
                    display_y += 10;
                }
            }
        }

        if (files_state.opened_file_size == 0)
        {
            graphics_draw_text(
                x + 10,
                y + 58,
                "Empty file.",
                ATLAS_COLOR_DARK_GRAY
            );
        }
    }
    else
    {
        /*
         * Directory list area.
         */
        graphics_fill_rect(
            x + 6,
            y + 48,
            width - 12,
            height - 72,
            ATLAS_COLOR_WHITE
        );

        graphics_draw_rect(
            x + 6,
            y + 48,
            width - 12,
            height - 72,
            ATLAS_COLOR_DARK_GRAY
        );

        /*
         * Column headers.
         */
        graphics_draw_text(
            x + 10,
            y + 53,
            "Name",
            ATLAS_COLOR_BLACK
        );

        graphics_draw_text(
            x + width - 55,
            y + 53,
            "Type",
            ATLAS_COLOR_BLACK
        );

        graphics_draw_line(
            x + 7,
            y + 64,
            x + width - 8,
            y + 64,
            ATLAS_COLOR_GRAY
        );

        /*
         * Directory entries.
         */
        if (files_state.entry_count == 0)
        {
            graphics_draw_text(
                x + 10,
                y + 72,
                "No items.",
                ATLAS_COLOR_DARK_GRAY
            );
        }
        else
        {
            uint32_t i;

            for (i = 0; i < files_state.entry_count; i++)
            {
                uint16_t entry_y;

                /*
                 * The first row begins below the column header.
                 */
                entry_y =
                    y + 69 +
                    (uint16_t)(i * 12);

                if (
                    entry_y + 8 >=
                    y + height - 21
                )
                {
                    break;
                }

                /*
                 * Highlight selected entry.
                 */
                if (
                    (int32_t)i ==
                    files_state.selected_index
                )
                {
                    graphics_fill_rect(
                        x + 7,
                        entry_y - 2,
                        width - 14,
                        11,
                        ATLAS_COLOR_BLUE
                    );

                    graphics_draw_text(
                        x + 10,
                        entry_y,
                        files_state.entries[i].name,
                        ATLAS_COLOR_WHITE
                    );
                }
                else
                {
                    graphics_draw_text(
                        x + 10,
                        entry_y,
                        files_state.entries[i].name,
                        ATLAS_COLOR_BLACK
                    );
                }

                /*
                 * Entry type.
                 */
                if (
                    files_state.entries[i].type ==
                    ATLASFS_TYPE_DIRECTORY
                )
                {
                    graphics_draw_text(
                        x + width - 55,
                        entry_y,
                        "DIR",
                        (int32_t)i ==
                        files_state.selected_index
                            ? ATLAS_COLOR_WHITE
                            : ATLAS_COLOR_DARK_GRAY
                    );
                }
                else
                {
                    graphics_draw_text(
                        x + width - 55,
                        entry_y,
                        "FILE",
                        (int32_t)i ==
                        files_state.selected_index
                            ? ATLAS_COLOR_WHITE
                            : ATLAS_COLOR_DARK_GRAY
                    );
                }
            }
        }
    }

    /*
     * Status bar.
     */
    graphics_fill_rect(
        x + 6,
        y + height - 19,
        width - 12,
        13,
        ATLAS_COLOR_GRAY
    );

    if (files_state.file_open != 0)
    {
        graphics_draw_text(
            x + 10,
            y + height - 15,
            "File opened",
            ATLAS_COLOR_BLACK
        );
    }
    else if (files_state.entry_count == 1)
    {
        graphics_draw_text(
            x + 10,
            y + height - 15,
            "1 item",
            ATLAS_COLOR_BLACK
        );
    }
    else
    {
        char status_text[16];
        uint32_t value;
        uint32_t position;
        uint32_t digit_count;

        value = files_state.entry_count;
        position = 0;

        /*
         * Convert the entry count to text.
         */
        if (value == 0)
        {
            status_text[position++] = '0';
        }
        else
        {
            digit_count = 0;

            while (value > 0)
            {
                value /= 10;
                digit_count++;
            }

            value = files_state.entry_count;

            while (digit_count > 0)
            {
                uint32_t divisor;
                uint32_t digit;

                divisor = 1;

                uint32_t j;

                for (
                    j = 1;
                    j < digit_count;
                    j++
                )
                {
                    divisor *= 10;
                }

                digit = value / divisor;

                status_text[position++] =
                    '0' + (char)digit;

                value %= divisor;
                digit_count--;
            }
        }

        status_text[position++] = ' ';
        status_text[position++] = 'i';
        status_text[position++] = 't';
        status_text[position++] = 'e';
        status_text[position++] = 'm';
        status_text[position++] = 's';
        status_text[position] = '\0';

        graphics_draw_text(
            x + 10,
            y + height - 15,
            status_text,
            ATLAS_COLOR_BLACK
        );
    }
}

static int files_open_file(int32_t index)
{
    uint32_t bytes_read = 0;

    if (index < 0 || (uint32_t)index >= files_state.entry_count)
    {
        files_state.last_error = ATLASFS_NOT_FOUND;
        return 0;
    }

    if (files_state.entries[index].type != ATLASFS_TYPE_FILE)
    {
        files_state.last_error = ATLASFS_IS_DIRECTORY;
        return 0;
    }

    if (files_state.entries[index].size > 8192)
    {
        files_state.last_error = ATLASFS_ERROR;
        return 0;
    }

    int result;

    result = filesystem_read_file(
        files_state.entries[index].name,
        files_state.opened_file_data,
        8192,
        &bytes_read
    );

    if (result != ATLASFS_SUCCESS)
    {
        files_state.last_error = result;
        return 0;
    }

    for (uint32_t i = 0; i < ATLASFS_MAX_FILENAME; i++)
    {
        files_state.opened_file_name[i] =
            files_state.entries[index].name[i];

        if (files_state.entries[index].name[i] == '\0')
        {
            break;
        }
    }

    files_state.opened_file_size = bytes_read;
    files_state.file_open = 1;
    files_state.last_error = ATLASFS_SUCCESS;

    return 1;
}

static void files_close_file(void)
{
    files_state.file_open = 0;
    files_state.opened_file_name[0] = '\0';
    files_state.opened_file_size = 0;
}

static void files_refresh(void)
{
    files_load_directory();
}

void files_handle_click(
    uint32_t application_id,
    int32_t mouse_x,
    int32_t mouse_y
)
{
    Application* application;
    Window* window;

    int32_t local_x;
    int32_t local_y;

    uint32_t index;
    uint16_t entry_y;

    application = application_get(application_id);

    if (application == 0)
    {
        return;
    }

    window = window_get(application->window_id);

    if (window == 0)
    {
        return;
    }

    /*
     * Only handle clicks for the Files application.
     */
    if (files_state.application_id != application_id)
    {
        return;
    }

    /*
     * Convert the screen coordinates into
     * coordinates relative to the window.
     */
    local_x = mouse_x - window->x;
    local_y = mouse_y - window->y;

    /*
     * Back button.
     */
    if (
        local_x >= 6 &&
        local_x < 46 &&
        local_y >= 29 &&
        local_y < 45
    )
    {
        files_go_back();
        window_manager_redraw();
        return;
    }

    /*
     * Refresh button.
     */
    if (
        local_x >= (int32_t)window->width - 56 &&
        local_x < (int32_t)window->width - 6 &&
        local_y >= 29 &&
        local_y < 45
    )
    {
        files_refresh();
        window_manager_redraw();
        return;
    }

    /*
     * Directory/file list.
     *
     * The first row begins at local Y = 69.
     * Each row is 12 pixels high.
     */
    if (
        local_x >= 7 &&
        local_x < (int32_t)window->width - 7 &&
        local_y >= 67 &&
        local_y < (int32_t)window->height - 21
    )
    {
        index =
            (uint32_t)((local_y - 67) / 12);

        if (index >= files_state.entry_count)
        {
            return;
        }

        entry_y =
            (uint16_t)(69 + (index * 12));

        /*
         * Make sure the click is actually inside
         * this row rather than in the gap between rows.
         */
        if (
            local_y < (int32_t)entry_y - 2 ||
            local_y >= (int32_t)entry_y + 9
        )
        {
            return;
        }

        /*
         * First click selects the item.
         */
        if (files_state.selected_index != (int32_t)index)
        {
            files_select_entry((int32_t)index);
            window_manager_redraw();
            return;
        }

        /*
         * Clicking the already-selected item activates it.
         */
        if (
            files_state.entries[index].type ==
            ATLASFS_TYPE_DIRECTORY
        )
        {
            int result;

            result = filesystem_change_directory(
                files_state.entries[index].name
            );

            if (result == ATLASFS_SUCCESS)
            {
                files_load_directory();
            }
            else
            {
                files_state.last_error = result;
            }

            window_manager_redraw();
            return;
        }

        /*
         * Open a regular file.
         */
        if (
            files_state.entries[index].type ==
            ATLASFS_TYPE_FILE
        )
        {
            files_open_file((int32_t)index);
            window_manager_redraw();
            return;
        }
    }
}

void files_shutdown(uint32_t application_id)
{
    if (files_state.application_id != application_id)
    {
        return;
    }

    files_close_file();

    files_state.application_id = 0;
    files_state.window_id = 0;
    files_state.entry_count = 0;
    files_state.selected_index = -1;
    files_state.scroll_offset = 0;
    files_state.last_error = ATLASFS_SUCCESS;
}