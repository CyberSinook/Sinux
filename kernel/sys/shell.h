#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdint.h>
#include <stddef.h>

/* NT-like Shell Definitions for Sinux */

/* Shell states */
#define SHELL_STATE_IDLE        0
#define SHELL_STATE_RUNNING     1
#define SHELL_STATE_EXIT        2

/* Maximum buffer sizes */
#define SHELL_COMMAND_MAX       256
#define SHELL_ARGS_MAX          16
#define SHELL_HISTORY_SIZE      32
#define SHELL_PATH_MAX          256

/* Command structure */
typedef struct {
    const char *name;                           /* Command name */
    const char *description;                    /* Command description */
    int (*handler)(int argc, char *argv[]);     /* Command handler function */
} shell_command_t;

/* Shell environment variable */
typedef struct {
    char name[64];
    char value[256];
} shell_env_var_t;

/* Shell context structure */
typedef struct {
    int state;                                  /* Shell state */
    char current_path[SHELL_PATH_MAX];          /* Current working directory */
    char command_buffer[SHELL_COMMAND_MAX];     /* Input command buffer */
    char *command_history[SHELL_HISTORY_SIZE];  /* Command history */
    int history_index;                          /* Current history index */
    int history_count;                          /* Total commands in history */
    shell_env_var_t *env_vars;                  /* Environment variables */
    int env_var_count;                          /* Number of env vars */
    int max_env_vars;                           /* Max env vars capacity */
    shell_command_t *commands;                  /* Registered commands */
    int command_count;                          /* Number of commands */
    int max_commands;                           /* Max commands capacity */
} shell_context_t;

/* Shell functions */
int shell_init(shell_context_t *shell);
int shell_register_command(shell_context_t *shell, shell_command_t *cmd);
int shell_execute_command(shell_context_t *shell, const char *input);
int shell_parse_command(const char *input, char *argv[], int max_args);
int shell_find_command(shell_context_t *shell, const char *name);
int shell_set_env_var(shell_context_t *shell, const char *name, const char *value);
char *shell_get_env_var(shell_context_t *shell, const char *name);
void shell_print_prompt(shell_context_t *shell);
void shell_add_history(shell_context_t *shell, const char *command);
void shell_run_interactive(shell_context_t *shell);
void shell_shutdown(shell_context_t *shell);

/* Built-in command handlers */
int shell_cmd_help(int argc, char *argv[]);
int shell_cmd_echo(int argc, char *argv[]);
int shell_cmd_clear(int argc, char *argv[]);
int shell_cmd_exit(int argc, char *argv[]);
int shell_cmd_history(int argc, char *argv[]);
int shell_cmd_set(int argc, char *argv[]);
int shell_cmd_get(int argc, char *argv[]);
int shell_cmd_ps(int argc, char *argv[]);
int shell_cmd_info(int argc, char *argv[]);
int shell_cmd_game(int argc, char *argv[]);

#endif /* __SHELL_H__ */
