#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

/*
 * AtlasFS
 * Neptune OS Atlas Build 006
 */

#define ATLASFS_MAGIC          0x41544653
#define ATLASFS_VERSION        1

#define ATLASFS_BLOCK_SIZE     512
#define ATLASFS_TOTAL_BLOCKS   32768

#define ATLASFS_RESERVED_BLOCK 0
#define ATLASFS_SUPERBLOCK     1

#define ATLASFS_BITMAP_START   2
#define ATLASFS_BITMAP_BLOCKS  8

#define ATLASFS_METADATA_START 10
#define ATLASFS_METADATA_BLOCKS 128

#define ATLASFS_DATA_START     138

#define ATLASFS_ROOT_METADATA_BLOCK ATLASFS_METADATA_START

#define ATLASFS_METADATA_SIZE \
    (ATLASFS_METADATA_BLOCKS * ATLASFS_BLOCK_SIZE)

#define ATLASFS_BITMAP_SIZE \
    (ATLASFS_BITMAP_BLOCKS * ATLASFS_BLOCK_SIZE)

#define ATLASFS_MAX_FILENAME   48
#define ATLASFS_MAX_PATH       256

/*
 * Filesystem object types.
 */
#define ATLASFS_TYPE_FREE      0
#define ATLASFS_TYPE_FILE      1
#define ATLASFS_TYPE_DIRECTORY 2

/*
 * Filesystem status codes.
 */
#define ATLASFS_SUCCESS        0
#define ATLASFS_ERROR          1
#define ATLASFS_NOT_FOUND      2
#define ATLASFS_ALREADY_EXISTS 3
#define ATLASFS_NO_SPACE       4
#define ATLASFS_INVALID_FS     5
#define ATLASFS_INVALID_PATH   6
#define ATLASFS_NOT_DIRECTORY  7
#define ATLASFS_IS_DIRECTORY   8
#define ATLASFS_NOT_EMPTY      9
#define ATLASFS_NOT_MOUNTED    -3
#define ATLASFS_INVALID_NAME   -7
#define ATLASFS_DIRECTORY_NOT_EMPTY -12

#define ATLASFS_MAX_DIRECTORY_ENTRIES 32
#define ATLASFS_INVALID_METADATA 0xFFFFFFFF

/*
 * AtlasFS superblock.
 */
typedef struct
{
    uint32_t magic;
    uint32_t version;

    uint32_t block_size;
    uint32_t total_blocks;

    uint32_t free_blocks;

    uint32_t bitmap_start;
    uint32_t bitmap_blocks;

    uint32_t metadata_start;
    uint32_t metadata_blocks;

    uint32_t data_start;

    uint32_t root_directory;

} AtlasSuperblock;

/*
 * File/directory metadata.
 */
typedef struct
{
    uint8_t  type;
    uint8_t  reserved[3];

    char     name[ATLASFS_MAX_FILENAME];

    uint32_t size;

    uint32_t start_block;

    uint32_t block_count;

    uint32_t parent;

} AtlasFileMetadata;

/*
 * Directory entry.
 */
typedef struct
{
    uint32_t metadata_block;

    char name[ATLASFS_MAX_FILENAME];

} AtlasDirectoryEntry;

/*
 * Filesystem runtime state.
 */
typedef struct
{
    AtlasSuperblock superblock;

    uint32_t current_directory;

    uint8_t mounted;

} AtlasFilesystem;

/*
 * Global filesystem instance.
 */
extern AtlasFilesystem atlas_filesystem;

/*
 * Filesystem initialization.
 */
void filesystem_initialize(void);

/*
 * Mount the filesystem.
 */
int filesystem_mount(void);

/*
 * Format the filesystem.
 */
int filesystem_format(void);

/*
 * Check whether the filesystem is mounted.
 */
int filesystem_is_mounted(void);

/*
 * Validate a filesystem superblock.
 */
int filesystem_validate_superblock(
    const AtlasSuperblock* superblock
);

int filesystem_create_file(
    const char* name
);

int filesystem_create_directory(
    const char* name
);

int filesystem_delete(
    const char* name
);

int filesystem_find(
    const char* name,
    AtlasFileMetadata* result
);

int filesystem_list_directory(void);

int filesystem_write_file(
    const char* name,
    const uint8_t* data,
    uint32_t size
);

int filesystem_read_file(
    const char* name,
    uint8_t* buffer,
    uint32_t buffer_size,
    uint32_t* bytes_read
);

const char* filesystem_get_current_directory(void);

int filesystem_change_directory(
    const char* path
);

int filesystem_remove_directory(
    const char* path
);

#endif