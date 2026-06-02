#include "shell.h"
#include <lib/inout.h>
#include <lib/string.h>
#include <kernel/arch/x86/mm/heap.h>
#include <kernel/games/pacman.h>

/* Maximum shell resources */
#define MAX_COMMANDS 32
#define MAX_ENV_VARS 64

/* Global shell context for command handlers */
static shell_context_t *g_shell_context = NULL;

/**
 * shell_print - Print string to serial console
 */
static void shell_print(const char *str)
{
    if (!str) return;
    for (const char *p = str; *p; p++) {
        outb(0x3F8, *p);
    }
}

/**
 * shell_init - Initialize shell context
 * @shell: Pointer to shell context
 * Returns: 0 on success, -1 on failure
 */
int shell_init(shell_context_t *shell)
{
    if (!shell) {
        return -1;
    }
    
    shell->state = SHELL_STATE_IDLE;
    shell->history_index = 0;
    shell->history_count = 0;
    shell->env_var_count = 0;
    shell->command_count = 0;
    shell->max_env_vars = MAX_ENV_VARS;
    shell->max_commands = MAX_COMMANDS;
    
    /* Allocate memory for environment variables */
    shell->env_vars = (shell_env_var_t *)malloc(sizeof(shell_env_var_t) * MAX_ENV_VARS);
    if (!shell->env_vars) {
        return -1;
    }
    
    /* Allocate memory for commands */
    shell->commands = (shell_command_t *)malloc(sizeof(shell_command_t) * MAX_COMMANDS);
    if (!shell->commands) {
        free(shell->env_vars);
        return -1;
    }
    
    /* Allocate memory for history */
    for (int i = 0; i < SHELL_HISTORY_SIZE; i++) {
        shell->command_history[i] = (char *)malloc(SHELL_COMMAND_MAX);
        if (!shell->command_history[i]) {
            return -1;
        }
    }
    
    /* Initialize current path */
    strcpy(shell->current_path, "C:\\");
    
    /* Set global context */
    g_shell_context = shell;
    
    shell_print("[SHELL] Shell initialized successfully\n");
    
    return 0;
}

/**
 * shell_register_command - Register a command with the shell
 * @shell: Pointer to shell context
 * @cmd: Pointer to command structure
 * Returns: 0 on success, -1 on failure
 */
int shell_register_command(shell_context_t *shell, shell_command_t *cmd)
{
    if (!shell || !cmd || shell->command_count >= MAX_COMMANDS) {
        return -1;
    }
    
    memcpy(&shell->commands[shell->command_count], cmd, sizeof(shell_command_t));
    shell->command_count++;
    
    return 0;
}

/**
 * shell_parse_command - Parse command line into arguments
 * @input: Input command string
 * @argv: Array to store parsed arguments
 * @max_args: Maximum number of arguments
 * Returns: Number of arguments parsed
 */
int shell_parse_command(const char *input, char *argv[], int max_args)
{
    if (!input || !argv) {
        return 0;
    }
    
    static char buffer[SHELL_COMMAND_MAX];
    strcpy(buffer, input);
    
    int argc = 0;
    char *token = buffer;
    
    while (*token && argc < max_args) {
        /* Skip whitespace */
        while (*token && (*token == ' ' || *token == '\t')) {
            token++;
        }
        
        if (!*token) break;
        
        argv[argc] = token;
        argc++;
        
        /* Find end of token */
        while (*token && *token != ' ' && *token != '\t') {
            token++;
        }
        
        if (*token) {
            *token = 0;
            token++;
        }
    }
    
    return argc;
}

/**
 * shell_find_command - Find a command by name
 * @shell: Pointer to shell context
 * @name: Command name
 * Returns: Command index or -1 if not found
 */
int shell_find_command(shell_context_t *shell, const char *name)
{
    if (!shell || !name) {
        return -1;
    }
    
    for (int i = 0; i < shell->command_count; i++) {
        if (strcmp(shell->commands[i].name, name) == 0) {
            return i;
        }
    }
    
    return -1;
}

/**
 * shell_set_env_var - Set environment variable
 * @shell: Pointer to shell context
 * @name: Variable name
 * @value: Variable value
 * Returns: 0 on success, -1 on failure
 */
int shell_set_env_var(shell_context_t *shell, const char *name, const char *value)
{
    if (!shell || !name || !value) {
        return -1;
    }
    
    /* Check if variable already exists */
    for (int i = 0; i < shell->env_var_count; i++) {
        if (strcmp(shell->env_vars[i].name, name) == 0) {
            strcpy(shell->env_vars[i].value, value);
            return 0;
        }
    }
    
    /* Add new variable */
    if (shell->env_var_count >= MAX_ENV_VARS) {
        return -1;
    }
    
    strcpy(shell->env_vars[shell->env_var_count].name, name);
    strcpy(shell->env_vars[shell->env_var_count].value, value);
    shell->env_var_count++;
    
    return 0;
}

/**
 * shell_get_env_var - Get environment variable value
 * @shell: Pointer to shell context
 * @name: Variable name
 * Returns: Variable value or NULL if not found
 */
char *shell_get_env_var(shell_context_t *shell, const char *name)
{
    if (!shell || !name) {
        return NULL;
    }
    
    for (int i = 0; i < shell->env_var_count; i++) {
        if (strcmp(shell->env_vars[i].name, name) == 0) {
            return shell->env_vars[i].value;
        }
    }
    
    return NULL;
}

/**
 * shell_print_prompt - Print shell prompt
 * @shell: Pointer to shell context
 */
void shell_print_prompt(shell_context_t *shell)
{
    if (!shell) return;
    
    shell_print(shell->current_path);
    shell_print("> ");
}

/**
 * shell_add_history - Add command to history
 * @shell: Pointer to shell context
 * @command: Command string
 */
void shell_add_history(shell_context_t *shell, const char *command)
{
    if (!shell || !command) return;
    
    int idx = shell->history_index % SHELL_HISTORY_SIZE;
    strcpy(shell->command_history[idx], command);
    
    shell->history_index++;
    if (shell->history_count < SHELL_HISTORY_SIZE) {
        shell->history_count++;
    }
}

/**
 * shell_execute_command - Parse and execute a command
 * @shell: Pointer to shell context
 * @input: Input command string
 * Returns: 0 on success, -1 on failure
 */
int shell_execute_command(shell_context_t *shell, const char *input)
{
    if (!shell || !input) {
        return -1;
    }
    
    /* Parse command */
    char *argv[SHELL_ARGS_MAX];
    int argc = shell_parse_command(input, argv, SHELL_ARGS_MAX);
    
    if (argc == 0) {
        return 0;
    }
    
    /* Add to history */
    shell_add_history(shell, input);
    
    /* Find and execute command */
    int cmd_idx = shell_find_command(shell, argv[0]);
    if (cmd_idx >= 0) {
        shell_command_t *cmd = &shell->commands[cmd_idx];
        if (cmd->handler) {
            return cmd->handler(argc, argv);
        }
    } else {
        shell_print("Command not found: ");
        shell_print(argv[0]);
        shell_print("\n");
        return -1;
    }
    
    return 0;
}

/* Built-in command implementations */

int shell_cmd_help(int argc, char *argv[])
{
    shell_print("\n=== SINUX SHELL COMMANDS ===\n");
    shell_print("help        - Show this help message\n");
    shell_print("echo        - Print text\n");
    shell_print("clear       - Clear screen\n");
    shell_print("exit        - Exit shell\n");
    shell_print("history     - Show command history\n");
    shell_print("set         - Set environment variable\n");
    shell_print("get         - Get environment variable\n");
    shell_print("ps          - List processes\n");
    shell_print("info        - Show system info\n");
    shell_print("game        - Launch Pacman game\n");
    shell_print("\n");
    return 0;
}

int shell_cmd_echo(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        shell_print(argv[i]);
        if (i < argc - 1) {
            shell_print(" ");
        }
    }
    shell_print("\n");
    return 0;
}

int shell_cmd_clear(int argc, char *argv[])
{
    shell_print("\033[2J\033[H");
    return 0;
}

int shell_cmd_exit(int argc, char *argv[])
{
    if (g_shell_context) {
        g_shell_context->state = SHELL_STATE_EXIT;
    }
    return 0;
}

int shell_cmd_history(int argc, char *argv[])
{
    if (!g_shell_context) return -1;
    
    shell_print("\n=== COMMAND HISTORY ===\n");
    for (int i = 0; i < g_shell_context->history_count; i++) {
        char num[16];
        int temp = i;
        int digits = 0;
        do {
            num[digits++] = (temp % 10) + '0';
            temp /= 10;
        } while (temp > 0);
        
        for (int j = digits - 1; j >= 0; j--) {
            outb(0x3F8, num[j]);
        }
        outb(0x3F8, ':');
        outb(0x3F8, ' ');
        shell_print(g_shell_context->command_history[i]);
        shell_print("\n");
    }
    shell_print("\n");
    return 0;
}

int shell_cmd_set(int argc, char *argv[])
{
    if (argc < 3) {
        shell_print("Usage: set <name> <value>\n");
        return -1;
    }
    
    if (!g_shell_context) return -1;
    
    if (shell_set_env_var(g_shell_context, argv[1], argv[2]) == 0) {
        shell_print("Variable set: ");
        shell_print(argv[1]);
        shell_print("=");
        shell_print(argv[2]);
        shell_print("\n");
        return 0;
    }
    
    shell_print("Failed to set variable\n");
    return -1;
}

int shell_cmd_get(int argc, char *argv[])
{
    if (argc < 2) {
        shell_print("Usage: get <name>\n");
        return -1;
    }
    
    if (!g_shell_context) return -1;
    
    char *value = shell_get_env_var(g_shell_context, argv[1]);
    if (value) {
        shell_print(argv[1]);
        shell_print("=");
        shell_print(value);
        shell_print("\n");
        return 0;
    }
    
    shell_print("Variable not found\n");
    return -1;
}

int shell_cmd_ps(int argc, char *argv[])
{
    shell_print("\n=== RUNNING PROCESSES ===\n");
    shell_print("PID   NAME\n");
    shell_print("1     init\n");
    shell_print("2     shell\n");
    shell_print("\n");
    return 0;
}

int shell_cmd_info(int argc, char *argv[])
{
    shell_print("\n=== SINUX SYSTEM INFORMATION ===\n");
    shell_print("OS Name:      Sinux\n");
    shell_print("Version:      0.03\n");
    shell_print("Architecture: x86\n");
    shell_print("Build Date:   2026-06-02\n");
    shell_print("\n");
    return 0;
}

int shell_cmd_game(int argc, char *argv[])
{
    shell_print("[SHELL] Launching Pacman game...\n");
    /* Game launch would be implemented here */
    return 0;
}

/**
 * shell_run_interactive - Run shell in interactive mode
 * @shell: Pointer to shell context
 */
void shell_run_interactive(shell_context_t *shell)
{
    if (!shell) return;
    
    shell->state = SHELL_STATE_RUNNING;
    
    shell_print("\n");
    shell_print("╔════════════════════════════════╗\n");
    shell_print("║  SINUX SHELL v0.03 (NT-Like)   ║\n");
    shell_print("║  Type 'help' for commands      ║\n");
    shell_print("╚════════════════════════════════╝\n");
    shell_print("\n");
    
    /* Register built-in commands */
    shell_command_t builtins[] = {
        {"help", "Show help message", shell_cmd_help},
        {"echo", "Print text", shell_cmd_echo},
        {"clear", "Clear screen", shell_cmd_clear},
        {"exit", "Exit shell", shell_cmd_exit},
        {"history", "Show command history", shell_cmd_history},
        {"set", "Set environment variable", shell_cmd_set},
        {"get", "Get environment variable", shell_cmd_get},
        {"ps", "List processes", shell_cmd_ps},
        {"info", "Show system info", shell_cmd_info},
        {"game", "Launch Pacman game", shell_cmd_game},
    };
    
    for (int i = 0; i < 10; i++) {
        shell_register_command(shell, &builtins[i]);
    }
    
    /* Main shell loop */
    while (shell->state == SHELL_STATE_RUNNING) {
        shell_print(shell->current_path);
        shell_print("> ");
        
        /* Simple command input simulation */
        /* In real implementation, would read from keyboard */
        for (int i = 0; i < 1000000; i++) {
            asm volatile("nop");
        }
    }
}

/**
 * shell_shutdown - Shutdown shell
 * @shell: Pointer to shell context
 */
void shell_shutdown(shell_context_t *shell)
{
    if (!shell) return;
    
    shell_print("\n[SHELL] Shutting down shell...\n");
    
    /* Free allocated memory */
    if (shell->env_vars) {
        free(shell->env_vars);
        shell->env_vars = NULL;
    }
    
    if (shell->commands) {
        free(shell->commands);
        shell->commands = NULL;
    }
    
    for (int i = 0; i < SHELL_HISTORY_SIZE; i++) {
        if (shell->command_history[i]) {
            free(shell->command_history[i]);
            shell->command_history[i] = NULL;
        }
    }
    
    shell->state = SHELL_STATE_EXIT;
    
    shell_print("[SHELL] Shell shutdown complete\n");
}
