#include "filesystem.h"
#include "disk.h"
#include "terminal.h"

AtlasFilesystem atlas_filesystem;

static int filesystem_read_bitmap_block(
    uint32_t bitmap_block,
    uint8_t* buffer
);

static int filesystem_write_bitmap_block(
    uint32_t bitmap_block,
    const uint8_t* buffer
);

static int filesystem_block_is_used(
    uint32_t block
);

static int filesystem_set_block_used(
    uint32_t block,
    uint8_t used
);

static int filesystem_find_free_blocks(
    uint32_t count,
    uint32_t* start_block
);

static int filesystem_allocate_blocks(
    uint32_t start_block,
    uint32_t count
);

static int filesystem_free_blocks(
    uint32_t start_block,
    uint32_t count
);

static uint32_t filesystem_blocks_required(
    uint32_t size
);

static int filesystem_update_free_blocks(
    int32_t change
);

static char filesystem_current_directory[256] = "/";


/*
 * Fill a memory region with a value.
 */
static void filesystem_memory_set(
    uint8_t* buffer,
    uint8_t value,
    uint32_t size
)
{
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        buffer[i] = value;
    }
}


/*
 * Set or clear one bit in the filesystem bitmap.
 *
 * 1 = used
 * 0 = free
 */
static void bitmap_set(
    uint8_t* bitmap,
    uint32_t block,
    uint8_t used
)
{
    uint32_t byte_index;
    uint8_t bit_index;

    byte_index = block / 8;
    bit_index = block % 8;

    if (used)
    {
        bitmap[byte_index] |=
            (uint8_t)(1 << bit_index);
    }
    else
    {
        bitmap[byte_index] &=
            (uint8_t)~(1 << bit_index);
    }
}


/*
 * Create the AtlasFS superblock.
 */
static void filesystem_create_superblock(
    AtlasSuperblock* superblock
)
{
    superblock->magic =
        ATLASFS_MAGIC;

    superblock->version =
        ATLASFS_VERSION;

    superblock->block_size =
        ATLASFS_BLOCK_SIZE;

    superblock->total_blocks =
        ATLASFS_TOTAL_BLOCKS;

    /*
     * Blocks 0 through 137 are reserved
     * for filesystem structures.
     *
     * Data begins at block 138.
     */
    superblock->free_blocks =
        ATLASFS_TOTAL_BLOCKS -
        ATLASFS_DATA_START;

    superblock->bitmap_start =
        ATLASFS_BITMAP_START;

    superblock->bitmap_blocks =
        ATLASFS_BITMAP_BLOCKS;

    superblock->metadata_start =
        ATLASFS_METADATA_START;

    superblock->metadata_blocks =
        ATLASFS_METADATA_BLOCKS;

    superblock->data_start =
        ATLASFS_DATA_START;

    superblock->root_directory =
        ATLASFS_ROOT_METADATA_BLOCK;
}


/*
 * Create the root directory metadata.
 */
static void filesystem_create_root(
    AtlasFileMetadata* root
)
{
    uint32_t i;

    root->type =
        ATLASFS_TYPE_DIRECTORY;

    for (i = 0; i < 3; i++)
    {
        root->reserved[i] = 0;
    }

    for (
        i = 0;
        i < ATLASFS_MAX_FILENAME;
        i++
    )
    {
        root->name[i] = 0;
    }

    root->name[0] = '/';

    root->size = 0;

    root->start_block = 0;

    root->block_count = 0;

    root->parent =
        ATLASFS_ROOT_METADATA_BLOCK;
}


/*
 * Write the superblock to disk.
 */
static int filesystem_write_superblock(
    const AtlasSuperblock* superblock
)
{
    uint8_t buffer[ATLASFS_BLOCK_SIZE];
    uint32_t i;

    filesystem_memory_set(
        buffer,
        0,
        ATLASFS_BLOCK_SIZE
    );

    for (
        i = 0;
        i < sizeof(AtlasSuperblock);
        i++
    )
    {
        buffer[i] =
            ((const uint8_t*)superblock)[i];
    }

    return disk_write_block(
        ATLASFS_SUPERBLOCK,
        buffer
    );
}


/*
 * Create and write the filesystem bitmap.
 *
 * The bitmap is eight blocks (4096 bytes),
 * allowing it to represent all 32768 blocks.
 */
static int filesystem_write_bitmap(void)
{
    uint8_t bitmap_block[ATLASFS_BLOCK_SIZE];

    uint32_t bitmap_block_index;
    uint32_t block;
    uint32_t block_start;
    uint32_t block_end;

    for (
        bitmap_block_index = 0;
        bitmap_block_index < ATLASFS_BITMAP_BLOCKS;
        bitmap_block_index++
    )
    {
        filesystem_memory_set(
            bitmap_block,
            0,
            ATLASFS_BLOCK_SIZE
        );

        /*
         * Each bitmap block represents
         * 4096 filesystem blocks.
         */
        block_start =
            bitmap_block_index * 4096;

        block_end =
            block_start + 4096;

        for (
            block = block_start;
            block < block_end &&
            block < ATLASFS_TOTAL_BLOCKS;
            block++
        )
        {
            bitmap_set(
                bitmap_block,
                block - block_start,
                block < ATLASFS_DATA_START
            );
        }

        if (
            disk_write_block(
                ATLASFS_BITMAP_START +
                bitmap_block_index,
                bitmap_block
            ) != DISK_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }
    }

    return ATLASFS_SUCCESS;
}


/*
 * Write the root directory metadata.
 */
static int filesystem_write_root(
    const AtlasFileMetadata* root
)
{
    uint8_t buffer[ATLASFS_BLOCK_SIZE];
    uint32_t i;

    filesystem_memory_set(
        buffer,
        0,
        ATLASFS_BLOCK_SIZE
    );

    for (
        i = 0;
        i < sizeof(AtlasFileMetadata);
        i++
    )
    {
        buffer[i] =
            ((const uint8_t*)root)[i];
    }

    return disk_write_block(
        ATLASFS_ROOT_METADATA_BLOCK,
        buffer
    );
}


/*
 * Initialize the filesystem subsystem.
 */
void filesystem_initialize(void)
{
    int result;

    result = filesystem_mount();

    if (result == ATLASFS_SUCCESS)
    {
        terminal_write(
            "Filesystem mounted.\n"
        );

        return;
    }

    terminal_write(
        "No valid filesystem found.\n"
    );

    terminal_write(
        "Formatting filesystem...\n"
    );

    result = filesystem_format();

    if (result != ATLASFS_SUCCESS)
    {
        terminal_write(
            "Filesystem format failed.\n"
        );

        return;
    }

    result = filesystem_mount();

    if (result != ATLASFS_SUCCESS)
    {
        terminal_write(
            "Filesystem mount failed.\n"
        );

        return;
    }

    terminal_write(
        "Filesystem mounted.\n"
    );
}

int filesystem_validate_superblock(
    const AtlasSuperblock* superblock
)
{
    if (superblock == 0)
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Check filesystem magic.
     */
    if (superblock->magic != ATLASFS_MAGIC)
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Check filesystem version.
     */
    if (superblock->version != ATLASFS_VERSION)
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Check block size.
     */
    if (superblock->block_size !=
        ATLASFS_BLOCK_SIZE)
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Check total disk size.
     */
    if (superblock->total_blocks !=
        ATLASFS_TOTAL_BLOCKS)
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Check bitmap location.
     */
    if (superblock->bitmap_start !=
        ATLASFS_BITMAP_START)
    {
        return ATLASFS_INVALID_FS;
    }

    if (superblock->bitmap_blocks !=
        ATLASFS_BITMAP_BLOCKS)
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Check metadata location.
     */
    if (superblock->metadata_start !=
        ATLASFS_METADATA_START)
    {
        return ATLASFS_INVALID_FS;
    }

    if (superblock->metadata_blocks !=
        ATLASFS_METADATA_BLOCKS)
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Check data region.
     */
    if (superblock->data_start !=
        ATLASFS_DATA_START)
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Check root directory location.
     */
    if (superblock->root_directory !=
        ATLASFS_ROOT_METADATA_BLOCK)
    {
        return ATLASFS_INVALID_FS;
    }

    return ATLASFS_SUCCESS;
}

/*
 * Mounting will be implemented in Step 6.
 */
int filesystem_mount(void)
{
    uint8_t buffer[ATLASFS_BLOCK_SIZE];
    AtlasSuperblock* superblock;

    /*
     * Read the AtlasFS superblock.
     */
    if (
        disk_read_block(
            ATLASFS_SUPERBLOCK,
            buffer
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    /*
     * Interpret the beginning of the block
     * as an AtlasSuperblock.
     */
    superblock =
        (AtlasSuperblock*)buffer;

    /*
     * Make sure this is actually AtlasFS.
     */
    if (
        filesystem_validate_superblock(
            superblock
        ) != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_INVALID_FS;
    }

    /*
     * Copy the superblock into the
     * runtime filesystem structure.
     */
    atlas_filesystem.superblock =
        *superblock;

    /*
     * Start at the root directory.
     */
    atlas_filesystem.current_directory =
        superblock->root_directory;

    /*
     * Filesystem is now mounted.
     */
    atlas_filesystem.mounted = 1;

    return ATLASFS_SUCCESS;
}


/*
 * Format the disk as AtlasFS.
 */
int filesystem_format(void)
{
    AtlasSuperblock superblock;
    AtlasFileMetadata root;

    /*
     * Build the superblock.
     */
    filesystem_create_superblock(
        &superblock
    );

    /*
     * Build the root directory.
     */
    filesystem_create_root(
        &root
    );

    /*
     * Write superblock.
     */
    if (
        filesystem_write_superblock(
            &superblock
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    /*
     * Write filesystem bitmap.
     */
    if (
        filesystem_write_bitmap()
        != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    /*
     * Write root directory metadata.
     */
    if (
        filesystem_write_root(
            &root
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    return ATLASFS_SUCCESS;
}


/*
 * Return whether AtlasFS is mounted.
 */
int filesystem_is_mounted(void)
{
    return atlas_filesystem.mounted;
}

static int filesystem_read_metadata(
    uint32_t metadata_block,
    AtlasFileMetadata* metadata
)
{
    uint8_t buffer[ATLASFS_BLOCK_SIZE];
    uint32_t i;

    if (
        disk_read_block(
            metadata_block,
            buffer
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    for (
        i = 0;
        i < sizeof(AtlasFileMetadata);
        i++
    )
    {
        ((uint8_t*)metadata)[i] =
            buffer[i];
    }

    return ATLASFS_SUCCESS;
}

static int filesystem_write_metadata(
    uint32_t metadata_block,
    const AtlasFileMetadata* metadata
)
{
    uint8_t buffer[ATLASFS_BLOCK_SIZE];
    uint32_t i;

    filesystem_memory_set(
        buffer,
        0,
        ATLASFS_BLOCK_SIZE
    );

    for (
        i = 0;
        i < sizeof(AtlasFileMetadata);
        i++
    )
    {
        buffer[i] =
            ((const uint8_t*)metadata)[i];
    }

    return disk_write_block(
        metadata_block,
        buffer
    );
}

static int filesystem_string_equals(
    const char* a,
    const char* b
)
{
    uint32_t i;

    if (a == 0 || b == 0)
    {
        return 0;
    }

    for (i = 0; i < ATLASFS_MAX_FILENAME; i++)
    {
        if (a[i] != b[i])
        {
            return 0;
        }

        if (a[i] == '\0')
        {
            return 1;
        }
    }

    return 1;
}

static int filesystem_find_free_metadata(
    uint32_t* result
)
{
    AtlasFileMetadata metadata;
    uint32_t block;

    for (
        block = ATLASFS_METADATA_START + 1;
        block <
        ATLASFS_METADATA_START +
        ATLASFS_METADATA_BLOCKS;
        block++
    )
    {
        if (
            filesystem_read_metadata(
                block,
                &metadata
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }

        if (metadata.type == 0)
        {
            *result = block;

            return ATLASFS_SUCCESS;
        }
    }

    return ATLASFS_NO_SPACE;
}

int filesystem_find(
    const char* name,
    AtlasFileMetadata* result
)
{
    AtlasFileMetadata metadata;
    uint32_t block;

    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    for (
        block = ATLASFS_METADATA_START;
        block <
        ATLASFS_METADATA_START +
        ATLASFS_METADATA_BLOCKS;
        block++
    )
    {
        if (
            filesystem_read_metadata(
                block,
                &metadata
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }

        if (metadata.type == 0)
        {
            continue;
        }

        if (
            metadata.parent !=
            atlas_filesystem.current_directory
        )
        {
            continue;
        }

        if (
            filesystem_string_equals(
                name,
                metadata.name
            )
        )
        {
            *result = metadata;

            return ATLASFS_SUCCESS;
        }
    }

    return ATLASFS_NOT_FOUND;
}

int filesystem_create_file(
    const char* name
)
{
    AtlasFileMetadata metadata;
    uint32_t metadata_block;
    uint32_t i;

    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    if (name == 0 || name[0] == '\0')
    {
        return ATLASFS_INVALID_NAME;
    }

    /*
     * Don't allow duplicate names.
     */
    if (
        filesystem_find(
            name,
            &metadata
        ) == ATLASFS_SUCCESS
    )
    {
        return ATLASFS_ALREADY_EXISTS;
    }

    /*
     * Find an unused metadata block.
     */
    if (
        filesystem_find_free_metadata(
            &metadata_block
        ) != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_NO_SPACE;
    }

    /*
     * Clear the metadata.
     */
    for (
        i = 0;
        i < sizeof(AtlasFileMetadata);
        i++
    )
    {
        ((uint8_t*)&metadata)[i] = 0;
    }

    metadata.type =
        ATLASFS_TYPE_FILE;

    /*
     * Copy filename.
     */
    for (
        i = 0;
        i < ATLASFS_MAX_FILENAME - 1 &&
        name[i] != '\0';
        i++
    )
    {
        metadata.name[i] = name[i];
    }

    metadata.name[i] = '\0';

    metadata.size = 0;

    metadata.start_block = 0;

    metadata.block_count = 0;

    metadata.parent =
        atlas_filesystem.current_directory;

    /*
     * Save metadata.
     */
    if (
        filesystem_write_metadata(
            metadata_block,
            &metadata
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    return ATLASFS_SUCCESS;
}

int filesystem_create_directory(
    const char* name
)
{
    AtlasFileMetadata metadata;
    uint32_t metadata_block;
    uint32_t i;

    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    if (name == 0 || name[0] == '\0')
    {
        return ATLASFS_INVALID_NAME;
    }

    /*
     * Check for duplicates.
     */
    if (
        filesystem_find(
            name,
            &metadata
        ) == ATLASFS_SUCCESS
    )
    {
        return ATLASFS_ALREADY_EXISTS;
    }

    /*
     * Find free metadata.
     */
    if (
        filesystem_find_free_metadata(
            &metadata_block
        ) != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_NO_SPACE;
    }

    /*
     * Clear metadata.
     */
    for (
        i = 0;
        i < sizeof(AtlasFileMetadata);
        i++
    )
    {
        ((uint8_t*)&metadata)[i] = 0;
    }

    metadata.type =
        ATLASFS_TYPE_DIRECTORY;

    /*
     * Copy directory name.
     */
    for (
        i = 0;
        i < ATLASFS_MAX_FILENAME - 1 &&
        name[i] != '\0';
        i++
    )
    {
        metadata.name[i] = name[i];
    }

    metadata.name[i] = '\0';

    metadata.size = 0;

    metadata.start_block = 0;

    metadata.block_count = 0;

    metadata.parent =
        atlas_filesystem.current_directory;

    /*
     * Write directory metadata.
     */
    if (
        filesystem_write_metadata(
            metadata_block,
            &metadata
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    return ATLASFS_SUCCESS;
}

int filesystem_list_directory(void)
{
    AtlasFileMetadata metadata;
    uint32_t block;
    
    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    for (
        block = ATLASFS_METADATA_START;
        block <
        ATLASFS_METADATA_START +
        ATLASFS_METADATA_BLOCKS;
        block++
    )
    {
        if (
            filesystem_read_metadata(
                block,
                &metadata
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }

        if (metadata.type == 0)
        {
            continue;
        }

        if (
            metadata.parent !=
            atlas_filesystem.current_directory
        )
        {
            continue;
        }

        if (
            metadata.type ==
            ATLASFS_TYPE_DIRECTORY
        )
        {
            terminal_write("[DIR] ");
        }
        else
        {
            terminal_write("[FILE] ");
        }

        terminal_write(metadata.name);

        terminal_write("\n");
    }

    return ATLASFS_SUCCESS;
}

int filesystem_delete(
    const char* name
)
{
    AtlasFileMetadata metadata;
    uint32_t block;

    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    for (
        block = ATLASFS_METADATA_START + 1;
        block <
        ATLASFS_METADATA_START +
        ATLASFS_METADATA_BLOCKS;
        block++
    )
    {
        if (
            filesystem_read_metadata(
                block,
                &metadata
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }

        if (metadata.type == 0)
        {
            continue;
        }

        if (
            metadata.parent !=
            atlas_filesystem.current_directory
        )
        {
            continue;
        }

        if (
            filesystem_string_equals(
                name,
                metadata.name
            )
        )
        {
            /*
             * Don't delete directories yet.
             * Directory deletion will need to verify
             * that they are empty.
             */
            if (
                metadata.type ==
                ATLASFS_TYPE_DIRECTORY
            )
            {
                return ATLASFS_IS_DIRECTORY;
            }

            /*
 * Release file data.
 */
            if (metadata.block_count > 0)
            {
                if (
                    filesystem_free_blocks(
                        metadata.start_block,
                        metadata.block_count
                    ) != ATLASFS_SUCCESS
                )
                {
                    return ATLASFS_ERROR;
                }
            }

            /*
             * Clear metadata to mark it unused.
             */
            filesystem_memory_set(
                (uint8_t*)&metadata,
                0,
                sizeof(AtlasFileMetadata)
            );

            if (
                filesystem_write_metadata(
                    block,
                    &metadata
                ) != DISK_SUCCESS
            )
            {
                return ATLASFS_ERROR;
            }

            return ATLASFS_SUCCESS;
        }
    }

    return ATLASFS_NOT_FOUND;
}

static int filesystem_read_bitmap_block(
    uint32_t bitmap_block,
    uint8_t* buffer
)
{
    if (
        bitmap_block >=
        ATLASFS_BITMAP_BLOCKS
    )
    {
        return ATLASFS_ERROR;
    }

    return disk_read_block(
        ATLASFS_BITMAP_START +
        bitmap_block,
        buffer
    );
}

static int filesystem_block_is_used(
    uint32_t block
)
{
    uint8_t bitmap_block[ATLASFS_BLOCK_SIZE];

    uint32_t bitmap_index;
    uint32_t byte_index;
    uint8_t bit_index;

    bitmap_index =
        block / 4096;

    byte_index =
        (block % 4096) / 8;

    bit_index =
        (uint8_t)((block % 4096) % 8);

    if (
        filesystem_read_bitmap_block(
            bitmap_index,
            bitmap_block
        ) != DISK_SUCCESS
    )
    {
        return -1;
    }

    return (
        bitmap_block[byte_index] &
        (uint8_t)(1 << bit_index)
    ) != 0;
}

static int filesystem_write_bitmap_block(
    uint32_t bitmap_block,
    const uint8_t* buffer
)
{
    if (
        bitmap_block >=
        ATLASFS_BITMAP_BLOCKS
    )
    {
        return ATLASFS_ERROR;
    }

    return disk_write_block(
        ATLASFS_BITMAP_START +
        bitmap_block,
        buffer
    );
}

static int filesystem_set_block_used(
    uint32_t block,
    uint8_t used
)
{
    uint8_t bitmap_block[ATLASFS_BLOCK_SIZE];

    uint32_t bitmap_index;
    uint32_t byte_index;
    uint8_t bit_index;

    bitmap_index =
        block / 4096;

    byte_index =
        (block % 4096) / 8;

    bit_index =
        (uint8_t)((block % 4096) % 8);

    if (
        filesystem_read_bitmap_block(
            bitmap_index,
            bitmap_block
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    if (used)
    {
        bitmap_block[byte_index] |=
            (uint8_t)(1 << bit_index);
    }
    else
    {
        bitmap_block[byte_index] &=
            (uint8_t)~(1 << bit_index);
    }

    if (
        filesystem_write_bitmap_block(
            bitmap_index,
            bitmap_block
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    return ATLASFS_SUCCESS;
}

static int filesystem_find_free_blocks(
    uint32_t count,
    uint32_t* start_block
)
{
    uint32_t block;
    uint32_t consecutive;

    if (count == 0)
    {
        return ATLASFS_ERROR;
    }

    consecutive = 0;

    for (
        block = ATLASFS_DATA_START;
        block < ATLASFS_TOTAL_BLOCKS;
        block++
    )
    {
        if (
            filesystem_block_is_used(
                block
            ) == 0
        )
        {
            consecutive++;

            if (consecutive == count)
            {
                *start_block =
                    block - count + 1;

                return ATLASFS_SUCCESS;
            }
        }
        else
        {
            consecutive = 0;
        }
    }

    return ATLASFS_NO_SPACE;
}

static int filesystem_allocate_blocks(
    uint32_t start_block,
    uint32_t count
)
{
    uint32_t block;

    for (
        block = start_block;
        block < start_block + count;
        block++
    )
    {
        if (
            filesystem_set_block_used(
                block,
                1
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }
    }

    if (
        filesystem_update_free_blocks(
            -(int32_t)count
        ) != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    return ATLASFS_SUCCESS;
}

static uint32_t filesystem_blocks_required(
    uint32_t size
)
{
    if (size == 0)
    {
        return 0;
    }

    return (
        (size + ATLASFS_BLOCK_SIZE - 1) /
        ATLASFS_BLOCK_SIZE
    );
}

int filesystem_write_file(
    const char* name,
    const uint8_t* data,
    uint32_t size
)
{
    AtlasFileMetadata metadata;

    uint32_t i;

    uint32_t old_start;
    uint32_t old_blocks;

    uint32_t new_start;
    uint32_t new_blocks;

    uint32_t block;
    uint32_t bytes_remaining;
    uint32_t bytes_to_write;

    uint8_t block_buffer[ATLASFS_BLOCK_SIZE];

    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    if (name == 0)
    {
        return ATLASFS_INVALID_NAME;
    }

    if (data == 0 && size != 0)
    {
        return ATLASFS_ERROR;
    }

    /*
     * Find the file.
     */
    if (
        filesystem_find(
            name,
            &metadata
        ) != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_NOT_FOUND;
    }

    if (
        metadata.type !=
        ATLASFS_TYPE_FILE
    )
    {
        return ATLASFS_IS_DIRECTORY;
    }

    /*
     * Remember the old allocation.
     */
    old_start =
        metadata.start_block;

    old_blocks =
        metadata.block_count;

    /*
     * Calculate new allocation.
     */
    new_blocks =
        filesystem_blocks_required(size);

    new_start = 0;

    /*
     * Find space for the new file.
     */
    if (new_blocks > 0)
    {
        if (
            filesystem_find_free_blocks(
                new_blocks,
                &new_start
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_NO_SPACE;
        }

        if (
            filesystem_allocate_blocks(
                new_start,
                new_blocks
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }
    }

    /*
     * Write file data.
     */
    bytes_remaining = size;

    for (
        block = 0;
        block < new_blocks;
        block++
    )
    {
        filesystem_memory_set(
            block_buffer,
            0,
            ATLASFS_BLOCK_SIZE
        );

        if (
            bytes_remaining >
            ATLASFS_BLOCK_SIZE
        )
        {
            bytes_to_write =
                ATLASFS_BLOCK_SIZE;
        }
        else
        {
            bytes_to_write =
                bytes_remaining;
        }

        for (
            i = 0;
            i < bytes_to_write;
            i++
        )
        {
            block_buffer[i] =
                data[
                    block *
                    ATLASFS_BLOCK_SIZE +
                    i
                ];
        }

        if (
            disk_write_block(
                new_start + block,
                block_buffer
            ) != DISK_SUCCESS
        )
        {
            /*
             * Undo the new allocation.
             */
            filesystem_free_blocks(
                new_start,
                new_blocks
            );

            return ATLASFS_ERROR;
        }

        bytes_remaining -=
            bytes_to_write;
    }

    /*
     * Release the old file allocation.
     */
    if (old_blocks > 0)
    {
        if (
            filesystem_free_blocks(
                old_start,
                old_blocks
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }
    }

    /*
     * Update metadata.
     */
    metadata.size = size;

    metadata.start_block =
        new_start;

    metadata.block_count =
        new_blocks;

    /*
     * Find the metadata block again
     * and update it.
     */
    {
        uint32_t metadata_block;

        for (
            metadata_block =
                ATLASFS_METADATA_START + 1;

            metadata_block <
                ATLASFS_METADATA_START +
                ATLASFS_METADATA_BLOCKS;

            metadata_block++
        )
        {
            AtlasFileMetadata existing;

            if (
                filesystem_read_metadata(
                    metadata_block,
                    &existing
                ) != ATLASFS_SUCCESS
            )
            {
                return ATLASFS_ERROR;
            }

            if (
                existing.type == 0
            )
            {
                continue;
            }

            if (
                existing.parent !=
                metadata.parent
            )
            {
                continue;
            }

            if (
                filesystem_string_equals(
                    existing.name,
                    metadata.name
                )
            )
            {
                if (
                    filesystem_write_metadata(
                        metadata_block,
                        &metadata
                    ) != DISK_SUCCESS
                )
                {
                    return ATLASFS_ERROR;
                }

                return ATLASFS_SUCCESS;
            }
        }
    }

    return ATLASFS_ERROR;
}

int filesystem_read_file(
    const char* name,
    uint8_t* buffer,
    uint32_t buffer_size,
    uint32_t* bytes_read
)
{
    AtlasFileMetadata metadata;

    uint32_t block;
    uint32_t bytes_remaining;
    uint32_t bytes_to_copy;

    uint8_t block_buffer[ATLASFS_BLOCK_SIZE];

    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    if (
        name == 0 ||
        buffer == 0 ||
        bytes_read == 0
    )
    {
        return ATLASFS_ERROR;
    }

    /*
     * Find the file.
     */
    if (
        filesystem_find(
            name,
            &metadata
        ) != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_NOT_FOUND;
    }

    if (
        metadata.type !=
        ATLASFS_TYPE_FILE
    )
    {
        return ATLASFS_IS_DIRECTORY;
    }

    /*
     * Don't overflow the caller's buffer.
     */
    if (buffer_size < metadata.size)
    {
        return ATLASFS_NO_SPACE;
    }

    bytes_remaining =
        metadata.size;

    *bytes_read = 0;

    for (
        block = 0;
        block < metadata.block_count;
        block++
    )
    {
        if (
            disk_read_block(
                metadata.start_block + block,
                block_buffer
            ) != DISK_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }

        if (
            bytes_remaining >
            ATLASFS_BLOCK_SIZE
        )
        {
            bytes_to_copy =
                ATLASFS_BLOCK_SIZE;
        }
        else
        {
            bytes_to_copy =
                bytes_remaining;
        }

        {
            uint32_t i;

            for (
                i = 0;
                i < bytes_to_copy;
                i++
            )
            {
                buffer[
                    block *
                    ATLASFS_BLOCK_SIZE +
                    i
                ] =
                    block_buffer[i];
            }
        }

        bytes_remaining -=
            bytes_to_copy;

        *bytes_read +=
            bytes_to_copy;
    }

    return ATLASFS_SUCCESS;
}

static int filesystem_update_free_blocks(
    int32_t change
)
{
    AtlasSuperblock superblock;
    uint8_t buffer[ATLASFS_BLOCK_SIZE];
    uint32_t i;

    if (
        disk_read_block(
            ATLASFS_SUPERBLOCK,
            buffer
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    for (
        i = 0;
        i < sizeof(AtlasSuperblock);
        i++
    )
    {
        ((uint8_t*)&superblock)[i] =
            buffer[i];
    }

    if (change < 0)
    {
        superblock.free_blocks -=
            (uint32_t)(-change);
    }
    else
    {
        superblock.free_blocks +=
            (uint32_t)change;
    }

    if (
        filesystem_write_superblock(
            &superblock
        ) != DISK_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    atlas_filesystem.superblock =
        superblock;

    return ATLASFS_SUCCESS;
}

static int filesystem_free_blocks(
    uint32_t start_block,
    uint32_t count
)
{
    uint32_t block;

    for (
        block = start_block;
        block < start_block + count;
        block++
    )
    {
        if (
            filesystem_set_block_used(
                block,
                0
            ) != ATLASFS_SUCCESS
        )
        {
            return ATLASFS_ERROR;
        }
    }

    if (
        filesystem_update_free_blocks(
            (int32_t)count
        ) != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_ERROR;
    }

    return ATLASFS_SUCCESS;
}

const char* filesystem_get_current_directory(void)
{
    return filesystem_current_directory;
}

int filesystem_change_directory(
    const char* path
)
{
    AtlasFileMetadata metadata;

    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    if (
        path == 0 ||
        path[0] == '\0'
    )
    {
        return ATLASFS_INVALID_PATH;
    }

    /*
     * Return to the filesystem root.
     */
    if (
        path[0] == '/' &&
        path[1] == '\0'
    )
    {
        filesystem_current_directory[0] = '/';
        filesystem_current_directory[1] = '\0';

        return ATLASFS_SUCCESS;
    }

    /*
     * Move to the parent directory.
     */
    if (
        path[0] == '.' &&
        path[1] == '.' &&
        path[2] == '\0'
    )
    {
        uint32_t length;
        uint32_t i;

        length = 0;

        while (
            filesystem_current_directory[length] != '\0'
        )
        {
            length++;
        }

        if (
            length <= 1
        )
        {
            return ATLASFS_SUCCESS;
        }

        /*
         * Remove trailing slash.
         */
        if (
            filesystem_current_directory[length - 1] == '/'
        )
        {
            length--;
        }

        i = length;

        while (
            i > 0 &&
            filesystem_current_directory[i - 1] != '/'
        )
        {
            i--;
        }

        if (i <= 1)
        {
            filesystem_current_directory[0] = '/';
            filesystem_current_directory[1] = '\0';
        }
        else
        {
            filesystem_current_directory[i - 1] = '\0';
        }

        return ATLASFS_SUCCESS;
    }

    /*
     * For now, only support directory names relative
     * to the current directory.
     */
    if (
        filesystem_find(
            path,
            &metadata
        ) != ATLASFS_SUCCESS
    )
    {
        return ATLASFS_NOT_FOUND;
    }

    if (
        metadata.type != ATLASFS_IS_DIRECTORY
    )
    {
        return ATLASFS_NOT_DIRECTORY;
    }

    /*
     * Root -> directory
     */
    if (
        filesystem_current_directory[0] == '/' &&
        filesystem_current_directory[1] == '\0'
    )
    {
        uint32_t length;
        uint32_t i;

        length = 0;

        while (path[length] != '\0')
        {
            length++;
        }

        if (
            length + 2 >=
            sizeof(filesystem_current_directory)
        )
        {
            return ATLASFS_INVALID_PATH;
        }

        filesystem_current_directory[0] = '/';

        for (
            i = 0;
            i < length;
            i++
        )
        {
            filesystem_current_directory[i + 1] =
                path[i];
        }

        filesystem_current_directory[length + 1] = '\0';

        return ATLASFS_SUCCESS;
    }

    return ATLASFS_SUCCESS;
}

int filesystem_remove_directory(
    const char* path
)
{
    AtlasFileMetadata metadata;

    int result;

    if (!filesystem_is_mounted())
    {
        return ATLASFS_NOT_MOUNTED;
    }

    result =
        filesystem_find(
            path,
            &metadata
        );

    if (
        result != ATLASFS_SUCCESS
    )
    {
        return result;
    }

    if (
        metadata.type != ATLASFS_IS_DIRECTORY
    )
    {
        return ATLASFS_NOT_DIRECTORY;
    }

    /*
     * The directory must be empty.
     *
     * Your existing directory-listing/metadata
     * implementation should be used here to determine
     * whether entries exist.
     */

    return filesystem_delete(path);
}