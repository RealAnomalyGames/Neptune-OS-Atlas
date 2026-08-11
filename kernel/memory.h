#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

typedef struct
{
    uint32_t lower_memory_kb;
    uint32_t upper_memory_kb;
    uint32_t total_memory_kb;

} MemoryInformation;

void memory_initialize(uint32_t multiboot_information);

void memory_get_information(
    MemoryInformation* information
);

uint32_t memory_get_total_kb(void);

#endif