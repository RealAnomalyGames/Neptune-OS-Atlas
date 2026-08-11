#include "memory.h"

static MemoryInformation memory_information;

void memory_initialize(uint32_t multiboot_information)
{
    /*
     * Multiboot information structure:
     *
     * Offset 0:
     *   flags
     *
     * Offset 4:
     *   lower memory
     *
     * Offset 8:
     *   upper memory
     */

    uint32_t* multiboot =
        (uint32_t*)multiboot_information;

    uint32_t flags =
        multiboot[0];

    memory_information.lower_memory_kb = 0;
    memory_information.upper_memory_kb = 0;
    memory_information.total_memory_kb = 0;

    /*
     * Bit 0 indicates that the memory
     * information fields are valid.
     */
    if ((flags & 0x01) == 0)
    {
        return;
    }

    memory_information.lower_memory_kb =
        multiboot[1];

    memory_information.upper_memory_kb =
        multiboot[2];

    /*
     * upper_memory_kb is memory above
     * the first 1 MiB.
     */
    memory_information.total_memory_kb =
        1024 +
        memory_information.upper_memory_kb;
}

void memory_get_information(
    MemoryInformation* information
)
{
    if (information == 0)
    {
        return;
    }

    *information = memory_information;
}

uint32_t memory_get_total_kb(void)
{
    return memory_information.total_memory_kb;
}