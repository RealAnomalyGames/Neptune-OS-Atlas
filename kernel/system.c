#include "system.h"
#include "cpu.h"
#include "memory.h"

void system_initialize(void)
{
    /*
     * System initialization will be expanded
     * as additional system modules are added.
     */
}

void system_get_information(SystemInformation* information)
{
    MemoryInformation memory;

    if (information == 0)
    {
        return;
    }

    information->name =
        ATLAS_NAME;

    information->version =
        ATLAS_VERSION;

    information->architecture =
        ATLAS_ARCHITECTURE;

    information->cpu =
        cpu_get_vendor();

    memory_get_information(&memory);

    information->memory_kb =
        memory.total_memory_kb;

    information->uptime_seconds =
        0;
}

const char* system_get_name(void)
{
    return ATLAS_NAME;
}

const char* system_get_version(void)
{
    return ATLAS_VERSION;
}

const char* system_get_architecture(void)
{
    return ATLAS_ARCHITECTURE;
}