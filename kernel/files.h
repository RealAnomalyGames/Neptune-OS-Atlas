#ifndef FILES_H
#define FILES_H

#include <stdint.h>

#include "filesystem.h"

#define FILES_MAX_ENTRIES ATLASFS_MAX_DIRECTORY_ENTRIES

typedef struct
{
    uint32_t metadata_block;
    char name[ATLASFS_MAX_FILENAME];
    uint8_t type;
    uint32_t size;
} FilesEntry;

typedef struct
{
    uint32_t application_id;
    uint32_t window_id;

    char current_path[ATLASFS_MAX_PATH];

    FilesEntry entries[FILES_MAX_ENTRIES];
    uint32_t entry_count;

    int32_t selected_index;
    uint32_t scroll_offset;

    int last_error;

    uint8_t file_open;
    char opened_file_name[ATLASFS_MAX_FILENAME];
    uint8_t opened_file_data[8192];
    uint32_t opened_file_size;

    uint8_t initialized;
} FilesState;

void files_manager_initialize(void);

int32_t files_launch(void);

void files_render(uint32_t application_id);

void files_handle_click(uint32_t application_id, int32_t mouse_x, int32_t mouse_y);

void files_shutdown(uint32_t application_id);

#endif