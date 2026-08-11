#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

#define ATLAS_NAME "Neptune OS Atlas"
#define ATLAS_VERSION "0.05"
#define ATLAS_BUILD 005
#define ATLAS_ARCHITECTURE "i386"

typedef struct
{
    const char* name;
    const char* version;
    uint32_t build;

    const char* architecture;

    const char* cpu;
    uint32_t memory_kb;
    uint32_t uptime_seconds;

} SystemInformation;

void system_initialize(void);

void system_get_information(SystemInformation* information);

const char* system_get_name(void);
const char* system_get_version(void);
const char* system_get_architecture(void);

#endif