#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "parser.h"
#include "cpu.h"
#include "memory.h"

static char shell_buffer[SHELL_BUFFER_SIZE];
static uint32_t shell_buffer_length;

static void shell_execute_command(ParsedCommand* command);

/*
 * Compare two strings.
 *
 * Returns:
 * 1 if the strings are equal
 * 0 if they are different
 */
static int shell_string_equals(
    const char* first,
    const char* second
)
{
    uint32_t i = 0;

    while (first[i] != '\0' && second[i] != '\0')
    {
        if (first[i] != second[i])
        {
            return 0;
        }

        i++;
    }

    return first[i] == '\0' && second[i] == '\0';
}

/*
 * Display the shell prompt.
 */
static void shell_print_prompt(void)
{
    terminal_write("> ");
}

/*
 * Execute the currently entered command.
 */
static void shell_execute_current_command(void)
{
    ParsedCommand command;

    parser_parse(shell_buffer, &command);

    shell_execute_command(&command);

    shell_clear_buffer();
}

/*
 * Initialize the shell.
 */
void shell_initialize(void)
{
    shell_buffer_length = 0;

    for (uint32_t i = 0; i < SHELL_BUFFER_SIZE; i++)
    {
        shell_buffer[i] = '\0';
    }

    terminal_write("NEPTUNE OS ATLAS\n");
    terminal_write("Build 005\n");
    terminal_write("\n");
}

/*
 * Run the shell.
 */
void shell_run(void)
{
    shell_print_prompt();

    while (1)
    {
        uint16_t key = keyboard_getkey();

        shell_process_key(key);
    }
}

/*
 * Clear the shell input buffer.
 */
void shell_clear_buffer(void)
{
    shell_buffer_length = 0;
    shell_buffer[0] = '\0';
}

/*
 * Add a character to the shell input buffer.
 */
void shell_add_character(char character)
{
    if (shell_buffer_length >= SHELL_BUFFER_SIZE - 1)
    {
        return;
    }

    shell_buffer[shell_buffer_length] = character;
    shell_buffer_length++;

    shell_buffer[shell_buffer_length] = '\0';
}

/*
 * Remove the last character from the shell input buffer.
 */
void shell_remove_character(void)
{
    if (shell_buffer_length == 0)
    {
        return;
    }

    shell_buffer_length--;

    shell_buffer[shell_buffer_length] = '\0';
}

/*
 * Get the current shell input buffer.
 */
const char* shell_get_buffer(void)
{
    return shell_buffer;
}

/*
 * Get the current shell input length.
 */
uint32_t shell_get_buffer_length(void)
{
    return shell_buffer_length;
}

/*
 * Process a keyboard key.
 */
void shell_process_key(uint16_t key)
{
    /*
     * Backspace
     */
    if (key == KEY_BACKSPACE)
    {
        if (shell_get_buffer_length() > 0)
        {
            shell_remove_character();
            terminal_backspace();
        }

        return;
    }

    /*
     * Enter
     */
    if (key == KEY_ENTER)
    {
        terminal_putchar('\n');

        if (shell_get_buffer_length() > 0)
        {
            shell_execute_current_command();
        }
        else
        {
            shell_clear_buffer();
        }

        shell_print_prompt();

        return;
    }

    /*
     * Tab
     */
    if (key == KEY_TAB)
    {
        for (int i = 0; i < 4; i++)
        {
            if (shell_get_buffer_length() < SHELL_BUFFER_SIZE - 1)
            {
                shell_add_character(' ');
                terminal_putchar(' ');
            }
        }

        return;
    }

    /*
     * Escape
     */
    if (key == KEY_ESCAPE)
    {
        return;
    }

    /*
     * Printable ASCII characters
     */
    if (key >= 32 && key <= 126)
    {
        shell_add_character((char)key);
        terminal_putchar((char)key);
    }
}

/*
 * Display the list of available commands.
 */
void shell_command_help(void)
{
    terminal_write("Available commands:\n");
    terminal_write("  help       Show available commands\n");
    terminal_write("  clear      Clear the screen\n");
    terminal_write("  about      About Neptune OS Atlas\n");
    terminal_write("  version    Show system version\n");
    terminal_write("  cpu        Show CPU information\n");
    terminal_write("  memory     Show memory information\n");
    terminal_write("  echo       Display text\n");
}

/*
 * Clear the terminal screen.
 */
void shell_command_clear(void)
{
    terminal_clear();
}

/*
 * Display information about Neptune OS Atlas.
 */
void shell_command_about(void)
{
    terminal_write("Neptune OS Atlas\n");
    terminal_write(
        "An experimental operating system developed by Neptune Corporation.\n"
    );
}

/*
 * Display the OS version.
 */
void shell_command_version(void)
{
    terminal_write("Neptune OS Atlas\n");
    terminal_write("Build 005\n");
    terminal_write("Architecture: x86\n");
}

void shell_command_cpu(void)
{
    CPUInformation information;

    cpu_get_information(&information);

    terminal_write("\nCPU Information\n");
    terminal_write("----------------\n");

    terminal_write("Vendor: ");
    terminal_write(information.vendor);
    terminal_write("\n");

    terminal_write("CPUID: ");

    if (information.cpuid_supported)
    {
        terminal_write("Supported");
    }
    else
    {
        terminal_write("Not supported");
    }

    terminal_write("\n");

    terminal_write("Maximum Basic Leaf: ");
    terminal_write_uint(information.maximum_basic_leaf);
    terminal_write("\n");

    terminal_write("Maximum Extended Leaf: ");
    terminal_write_uint(information.maximum_extended_leaf);
    terminal_write("\n");
}

void shell_command_memory(void)
{
    MemoryInformation information;

    memory_get_information(&information);

    terminal_write("\nMemory Information\n");
    terminal_write("------------------\n");

    terminal_write("Lower Memory: ");
    terminal_write_uint(
        information.lower_memory_kb
    );
    terminal_write(" KB\n");

    terminal_write("Upper Memory: ");
    terminal_write_uint(
        information.upper_memory_kb
    );
    terminal_write(" KB\n");

    terminal_write("Total Memory: ");
    terminal_write_uint(
        information.total_memory_kb
    );
    terminal_write(" KB\n");
}

/*
 * Display the supplied arguments.
 */
void shell_command_echo(ParsedCommand* command)
{
    for (uint32_t i = 0; i < command->argument_count; i++)
    {
        terminal_write(command->arguments[i]);

        if (i + 1 < command->argument_count)
        {
            terminal_putchar(' ');
        }
    }

    terminal_putchar('\n');
}

void shell_command_not_found(const char* command)
{
    terminal_write("Unknown command: ");
    terminal_write(command);
    terminal_putchar('\n');

    terminal_write("Type 'help' for a list of commands.\n");
}

/*
 * Determine which built-in command to execute.
 */
static void shell_execute_command(ParsedCommand* command)
{
    if (shell_string_equals(command->command, "help"))
    {
        shell_command_help();
        return;
    }

    if (shell_string_equals(command->command, "clear"))
    {
        shell_command_clear();
        return;
    }

    if (shell_string_equals(command->command, "about"))
    {
        shell_command_about();
        return;
    }

    if (shell_string_equals(command->command, "version"))
    {
        shell_command_version();
        return;
    }

    else if (shell_string_equals(command->command, "cpu"))
    {
        shell_command_cpu();
        return;
    }

    else if (shell_string_equals(command->command, "cpuinfo"))
    {
        shell_command_cpu();
        return;
    }

    else if (shell_string_equals(command->command, "memory"))
    {
        shell_command_memory();
        return;
    }

    if (shell_string_equals(command->command, "echo"))
    {
        shell_command_echo(command);
        return;
    }

    shell_command_not_found(command->command);
}