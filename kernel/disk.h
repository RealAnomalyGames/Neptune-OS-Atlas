#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define DISK_BLOCK_SIZE 512

#define DISK_SUCCESS 0
#define DISK_ERROR    1

void disk_initialize(void);

int disk_read_block(uint32_t block, uint8_t* buffer);

int disk_write_block(uint32_t block, const uint8_t* buffer);

uint32_t disk_get_block_count(void);

#endif