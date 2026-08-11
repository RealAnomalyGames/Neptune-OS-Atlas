#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "parser.h"

#define SHELL_BUFFER_SIZE 128

void shell_initialize(void);
void shell_run(void);

void shell_clear_buffer(void);

void shell_add_character(char character);
void shell_remove_character(void);

const char* shell_get_buffer(void);
uint32_t shell_get_buffer_length(void);

void shell_process_key(uint16_t key);

void shell_command_help(void);
void shell_command_clear(void);
void shell_command_about(void);
void shell_command_version(void);
void shell_command_cpu(void);
void shell_command_echo(ParsedCommand* command);
void shell_command_not_found(const char* command);

#endif