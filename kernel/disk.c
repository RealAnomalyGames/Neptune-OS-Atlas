#include "disk.h"
#include "io.h"

#define ATA_PRIMARY_DATA       0x1F0
#define ATA_PRIMARY_ERROR      0x1F1
#define ATA_PRIMARY_SECTOR     0x1F2
#define ATA_PRIMARY_LBA_LOW    0x1F3
#define ATA_PRIMARY_LBA_MID    0x1F4
#define ATA_PRIMARY_LBA_HIGH   0x1F5
#define ATA_PRIMARY_DRIVE      0x1F6
#define ATA_PRIMARY_COMMAND    0x1F7
#define ATA_PRIMARY_STATUS     0x1F7

#define ATA_CMD_READ_SECTORS   0x20
#define ATA_CMD_WRITE_SECTORS  0x30

#define ATA_STATUS_BSY         0x80
#define ATA_STATUS_DRDY        0x40
#define ATA_STATUS_DRQ         0x08
#define ATA_STATUS_ERR         0x01

#define DISK_TOTAL_BLOCKS      32768

static int ata_wait_for_ready(void)
{
    uint8_t status;

    do
    {
        status = inb(ATA_PRIMARY_STATUS);

        if (status & ATA_STATUS_ERR)
        {
            return DISK_ERROR;
        }

    } while (status & ATA_STATUS_BSY);

    if (!(status & ATA_STATUS_DRQ))
    {
        return DISK_ERROR;
    }

    return DISK_SUCCESS;
}

void disk_initialize(void)
{
}

int disk_read_block(uint32_t block, uint8_t* buffer)
{
    uint32_t lba;
    uint16_t i;

    if (buffer == 0)
    {
        return DISK_ERROR;
    }

    lba = block;

    outb(
        ATA_PRIMARY_DRIVE,
        0xE0 | ((lba >> 24) & 0x0F)
    );

    outb(ATA_PRIMARY_SECTOR, 1);

    outb(
        ATA_PRIMARY_LBA_LOW,
        (uint8_t)(lba & 0xFF)
    );

    outb(
        ATA_PRIMARY_LBA_MID,
        (uint8_t)((lba >> 8) & 0xFF)
    );

    outb(
        ATA_PRIMARY_LBA_HIGH,
        (uint8_t)((lba >> 16) & 0xFF)
    );

    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_READ_SECTORS
    );

    if (ata_wait_for_ready() != DISK_SUCCESS)
    {
        return DISK_ERROR;
    }

    for (i = 0; i < 256; i++)
    {
        uint16_t value;

        value = inw(ATA_PRIMARY_DATA);

        buffer[i * 2] =
            (uint8_t)(value & 0xFF);

        buffer[i * 2 + 1] =
            (uint8_t)((value >> 8) & 0xFF);
    }

    return DISK_SUCCESS;
}

int disk_write_block(
    uint32_t block,
    const uint8_t* buffer
)
{
    uint32_t lba;
    uint16_t i;

    if (buffer == 0)
    {
        return DISK_ERROR;
    }

    lba = block;

    outb(
        ATA_PRIMARY_DRIVE,
        0xE0 | ((lba >> 24) & 0x0F)
    );

    outb(ATA_PRIMARY_SECTOR, 1);

    outb(
        ATA_PRIMARY_LBA_LOW,
        (uint8_t)(lba & 0xFF)
    );

    outb(
        ATA_PRIMARY_LBA_MID,
        (uint8_t)((lba >> 8) & 0xFF)
    );

    outb(
        ATA_PRIMARY_LBA_HIGH,
        (uint8_t)((lba >> 16) & 0xFF)
    );

    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_WRITE_SECTORS
    );

    if (ata_wait_for_ready() != DISK_SUCCESS)
    {
        return DISK_ERROR;
    }

    for (i = 0; i < 256; i++)
    {
        uint16_t value;

        value =
            (uint16_t)buffer[i * 2]
            |
            ((uint16_t)buffer[i * 2 + 1] << 8);

        outw(
            ATA_PRIMARY_DATA,
            value
        );
    }

    return DISK_SUCCESS;
}

uint32_t disk_get_block_count(void)
{
    return DISK_TOTAL_BLOCKS;
}