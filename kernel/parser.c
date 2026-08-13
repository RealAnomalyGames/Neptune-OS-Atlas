#include "parser.h"

static int parser_is_space(char character)
{
    return character == ' ' ||
           character == '\t';
}

void parser_parse(char* input, ParsedCommand* result)
{
    uint32_t position = 0;

    result->command[0] = '\0';
    result->argument_count = 0;

    for (uint32_t i = 0; i < PARSER_MAX_ARGUMENTS; i++)
    {
        result->arguments[i] = 0;
    }

    /*
     * Skip leading whitespace.
     */
    while (parser_is_space(input[position]))
    {
        position++;
    }

    /*
     * Empty command.
     */
    if (input[position] == '\0')
    {
        return;
    }

    /*
     * The first word is the command.
     */
    uint32_t command_position = 0;

    while (
        input[position] != '\0' &&
        !parser_is_space(input[position])
    )
    {
        if (command_position < PARSER_MAX_COMMAND_LENGTH - 1)
        {
            result->command[command_position] =
                input[position];

            command_position++;
        }

        position++;
    }

    result->command[command_position] = '\0';

    /*
     * Skip whitespace after the command.
     */
    while (parser_is_space(input[position]))
    {
        position++;
    }

    /*
     * Parse arguments.
     */
    while (
        input[position] != '\0' &&
        result->argument_count < PARSER_MAX_ARGUMENTS
    )
    {
        result->arguments[result->argument_count] =
            &input[position];

        result->argument_count++;

        /*
         * Find the end of this argument.
         */
        while (
            input[position] != '\0' &&
            !parser_is_space(input[position])
        )
        {
            position++;
        }

        /*
         * Replace whitespace with a string terminator.
         */
        if (input[position] != '\0')
        {
            input[position] = '\0';
            position++;
        }

        /*
         * Skip additional whitespace.
         */
        while (
            input[position] == ' '
        )
        {
            position++;
        }
    }
}