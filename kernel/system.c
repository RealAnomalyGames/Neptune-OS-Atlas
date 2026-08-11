#include "system.h"

void system_initialize(void)
{
    /*
     * System initialization will be expanded
     * as additional system modules are added.
     */
}

void system_get_information(SystemInformation* information)
{
    if (information == 0)
    {
        return;
    }

    information->name = ATLAS_NAME;
    information->version = ATLAS_VERSION;
    information->architecture = ATLAS_ARCHITECTURE;

    /*
     * These will be populated by the CPU,
     * memory, and timer modules later.
     */
    information->cpu = "Unknown";
    information->memory_kb = 0;
    information->uptime_seconds = 0;
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