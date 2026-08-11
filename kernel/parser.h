#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#define PARSER_MAX_ARGUMENTS 16
#define PARSER_MAX_COMMAND_LENGTH 128

typedef struct
{
    char command[PARSER_MAX_COMMAND_LENGTH];
    char* arguments[PARSER_MAX_ARGUMENTS];
    uint32_t argument_count;
} ParsedCommand;

void parser_parse(char* input, ParsedCommand* result);

#endif