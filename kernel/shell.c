#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "parser.h"
#include "cpu.h"
#include "memory.h"
#include "timer.h"
#include "system.h"
#include "filesystem.h"

#define SHELL_FILE_BUFFER_SIZE 4096

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
    terminal_write("Build 007\n");
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
    terminal_write("  help                Show available commands\n");
    terminal_write("  clear               Clear the screen\n");
    terminal_write("  about               About Neptune OS Atlas\n");
    terminal_write("  version             Show system version\n");
    terminal_write("  cpu                 Show CPU information\n");
    terminal_write("  memory              Show memory information\n");
    terminal_write("  uptime              Show system uptime\n");
    terminal_write("  uname               Show system information\n");
    terminal_write("  sysinfo             Show detailed system information\n");
    terminal_write("  ls                  List directory contents\n");
    terminal_write("  touch <file>        Create a file\n");
    terminal_write("  mkdir <dir>         Create a directory\n");
    terminal_write("  cat <file>          Display a file\n");
    terminal_write("  write <file> <text> Write text to a file\n");
    terminal_write("  rm <file>           Delete a file\n");
    terminal_write("  cd <dir>            Change directory\n");
    terminal_write("  pwd                 Show current directory\n");
    terminal_write("  rmdir <dor>         Remove empty directory\n");
    terminal_write("  echo                Display text\n");
    
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

void shell_command_uptime(void)
{
    uint32_t seconds;

    seconds = timer_get_uptime_seconds();

    terminal_write("\nUptime: ");
    terminal_write_uint(seconds);
    terminal_write(" seconds\n");
}

void shell_command_uname(ParsedCommand* command)
{
    if (command == 0)
    {
        return;
    }

    if (command->argument_count == 0)
    {
        terminal_write(system_get_name());
        terminal_write("\n");

        return;
    }

    if (shell_string_equals(command->arguments[0], "-a"))
    {
        terminal_write(system_get_name());
        terminal_write(" ");
        terminal_write(system_get_version());
        terminal_write(" ");
        terminal_write(system_get_architecture());
        terminal_write("\n");

        return;
    }

    if (shell_string_equals(command->arguments[0], "-s"))
    {
        terminal_write(system_get_name());
        terminal_write("\n");

        return;
    }

    if (shell_string_equals(command->arguments[0], "-r"))
    {
        terminal_write(system_get_version());
        terminal_write("\n");

        return;
    }

    if (shell_string_equals(command->arguments[0], "-m"))
    {
        terminal_write(system_get_architecture());
        terminal_write("\n");

        return;
    }

    terminal_write("uname: invalid option\n");
    terminal_write("Usage: uname [-a] [-s] [-r] [-m]\n");
}

void shell_command_sysinfo(void)
{
    SystemInformation information;

    system_get_information(&information);

    terminal_write("\n");
    terminal_write(information.name);
    terminal_write("\n");

    terminal_write("========================\n");

    terminal_write("Version:       ");
    terminal_write(information.version);
    terminal_write("\n");

    terminal_write("Build:         ");
    terminal_write_uint(information.build);
    terminal_write("\n");

    terminal_write("Architecture:  ");
    terminal_write(information.architecture);
    terminal_write("\n");

    terminal_write("CPU:           ");
    terminal_write(information.cpu);
    terminal_write("\n");

    terminal_write("Memory:        ");
    terminal_write_uint(information.memory_kb);
    terminal_write(" KB\n");

    terminal_write("Uptime:        ");
    terminal_write_uint(information.uptime_seconds);
    terminal_write(" seconds\n");

    terminal_write("\n");
}

static void shell_command_ls(void)
{
    int result;

    result = filesystem_list_directory();

    if (result != ATLASFS_SUCCESS)
    {
        terminal_write("ls: failed to list directory.\n");
    }
}

static void shell_command_touch(
    const char* name
)
{
    int result;

    if (
        name == 0 ||
        name[0] == '\0'
    )
    {
        terminal_write(
            "touch: missing file name.\n"
        );

        return;
    }

    result =
        filesystem_create_file(name);

    if (result == ATLASFS_SUCCESS)
    {
        return;
    }

    if (result == ATLASFS_ALREADY_EXISTS)
    {
        terminal_write(
            "touch: file already exists.\n"
        );

        return;
    }

    if (result == ATLASFS_NO_SPACE)
    {
        terminal_write(
            "touch: filesystem is full.\n"
        );

        return;
    }

    terminal_write(
        "touch: failed to create file.\n"
    );
}

static void shell_command_mkdir(
    const char* name
)
{
    int result;

    if (
        name == 0 ||
        name[0] == '\0'
    )
    {
        terminal_write(
            "mkdir: missing directory name.\n"
        );

        return;
    }

    result =
        filesystem_create_directory(name);

    if (result == ATLASFS_SUCCESS)
    {
        return;
    }

    if (result == ATLASFS_ALREADY_EXISTS)
    {
        terminal_write(
            "mkdir: directory already exists.\n"
        );

        return;
    }

    if (result == ATLASFS_NO_SPACE)
    {
        terminal_write(
            "mkdir: filesystem is full.\n"
        );

        return;
    }

    terminal_write(
        "mkdir: failed to create directory.\n"
    );
}

static void shell_command_cat(
    const char* name
)
{
    uint8_t buffer[SHELL_FILE_BUFFER_SIZE];
    uint32_t bytes_read;
    uint32_t i;
    int result;

    if (
        name == 0 ||
        name[0] == '\0'
    )
    {
        terminal_write(
            "cat: missing file name.\n"
        );

        return;
    }

    result =
        filesystem_read_file(
            name,
            buffer,
            SHELL_FILE_BUFFER_SIZE,
            &bytes_read
        );

    if (result != ATLASFS_SUCCESS)
    {
        terminal_write(
            "cat: unable to read file.\n"
        );

        return;
    }

    for (
        i = 0;
        i < bytes_read;
        i++
    )
    {
        char character[2];

        character[0] =
            (char)buffer[i];

        character[1] =
            '\0';

        terminal_write(character);
    }

    terminal_write("\n");
}

static void shell_command_write(
    const char* name,
    const char* text
)
{
    int result;
    uint32_t length;

    if (
        name == 0 ||
        name[0] == '\0'
    )
    {
        terminal_write(
            "write: missing file name.\n"
        );

        return;
    }

    if (
        text == 0 ||
        text[0] == '\0'
    )
    {
        terminal_write(
            "write: missing text.\n"
        );

        return;
    }

    length = 0;

    while (
        text[length] != '\0'
    )
    {
        length++;
    }

    result =
        filesystem_write_file(
            name,
            (const uint8_t*)text,
            length
        );

    if (result == ATLASFS_SUCCESS)
    {
        return;
    }

    if (result == ATLASFS_NOT_FOUND)
    {
        terminal_write(
            "write: file not found.\n"
        );

        return;
    }

    if (result == ATLASFS_NO_SPACE)
    {
        terminal_write(
            "write: not enough space.\n"
        );

        return;
    }

    terminal_write(
        "write: failed to write file.\n"
    );
}

static void shell_command_rm(
    const char* name
)
{
    int result;

    if (
        name == 0 ||
        name[0] == '\0'
    )
    {
        terminal_write(
            "rm: missing file name.\n"
        );

        return;
    }

    result =
        filesystem_delete(name);

    if (result == ATLASFS_SUCCESS)
    {
        return;
    }

    if (result == ATLASFS_NOT_FOUND)
    {
        terminal_write(
            "rm: file not found.\n"
        );

        return;
    }

    if (result == ATLASFS_IS_DIRECTORY)
    {
        terminal_write(
            "rm: cannot remove directory.\n"
        );

        return;
    }

    terminal_write(
        "rm: failed to remove file.\n"
    );
}

static void shell_command_pwd(void)
{
    terminal_write(
        filesystem_get_current_directory()
    );

    terminal_write("\n");
}

static void shell_command_cd(
    const char* path
)
{
    int result;

    if (
        path == 0 ||
        path[0] == '\0'
    )
    {
        path = "/";
    }

    result =
        filesystem_change_directory(path);

    if (
        result == ATLASFS_SUCCESS
    )
    {
        return;
    }

    if (
        result == ATLASFS_NOT_FOUND
    )
    {
        terminal_write(
            "cd: directory not found.\n"
        );

        return;
    }

    if (
        result == ATLASFS_NOT_DIRECTORY
    )
    {
        terminal_write(
            "cd: not a directory.\n"
        );

        return;
    }

    terminal_write(
        "cd: invalid path.\n"
    );
}

static void shell_command_rmdir(
    const char* name
)
{
    int result;

    if (
        name == 0 ||
        name[0] == '\0'
    )
    {
        terminal_write(
            "rmdir: missing directory name.\n"
        );

        return;
    }

    result =
        filesystem_remove_directory(name);

    if (
        result == ATLASFS_SUCCESS
    )
    {
        return;
    }

    if (
        result == ATLASFS_NOT_FOUND
    )
    {
        terminal_write(
            "rmdir: directory not found.\n"
        );

        return;
    }

    if (
        result == ATLASFS_NOT_DIRECTORY
    )
    {
        terminal_write(
            "rmdir: not a directory.\n"
        );

        return;
    }

    if (
        result == ATLASFS_DIRECTORY_NOT_EMPTY
    )
    {
        terminal_write(
            "rmdir: directory not empty.\n"
        );

        return;
    }

    terminal_write(
        "rmdir: failed to remove directory.\n"
    );
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

    else if (shell_string_equals(command->command, "uptime"))
    {
        shell_command_uptime();
        return;
    }

    else if (shell_string_equals(command->command, "uname"))
    {
        shell_command_uname(command);
        return;
    }

    else if (shell_string_equals(command->command, "sysinfo"))
    {
        shell_command_sysinfo();
        return;
    }

    else if (shell_string_equals(command->command, "ls"))
    {
        shell_command_ls();
        return;
    }

    if (shell_string_equals(command->command, "touch"))
    {
        shell_command_touch(command->arguments[0]);
        return;
    }

    else if (shell_string_equals(command->command, "mkdir"))
    {
        shell_command_mkdir(command->arguments[0]);
        return;
    }

    if (shell_string_equals(command->command, "cat"))
    {
        shell_command_cat(command->arguments[0]);
        return;
    }

    else if (shell_string_equals(command->command, "rm"))
    {
        shell_command_rm(command->arguments[0]);
        return;
    }

    if (shell_string_equals(command->command, "write"))
    {
        shell_command_write(command->arguments[0], command->arguments[1]);
        return;
    }

    if (shell_string_equals(command->command, "pwd"))
    {
        shell_command_pwd();
        return;
    }

    if (shell_string_equals(command->command, "cd"))
    {
        shell_command_cd(command->arguments[0]);
        return;
    }

    if (shell_string_equals(command->command, "rmdir"))
    {
        shell_command_rmdir(command->arguments[0]);
        return;
    }

    if (shell_string_equals(command->command, "echo"))
    {
        shell_command_echo(command);
        return;
    }

    shell_command_not_found(command->command);
}