#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

#define SHELL_BUFFER_SIZE 128

void shell_initialize(void);
void shell_clear_buffer(void);

void shell_add_character(char character);
void shell_remove_character(void);

const char* shell_get_buffer(void);
uint32_t shell_get_buffer_length(void);

#endif