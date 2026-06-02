# NT-like Shell for Sinux OS

This directory contains the shell implementation for Sinux OS.

## Files

- **shell.h** - Shell header with command definitions and structures
- **shell.c** - Shell implementation with command parsing and execution

## Commands

### Built-in Commands

- `help` - Display help message with available commands
- `echo [text]` - Print text to console
- `clear` - Clear the screen
- `exit` - Exit the shell
- `history` - Show command history
- `set [name] [value]` - Set environment variable
- `get [name]` - Get environment variable value
- `ps` - List running processes
- `info` - Show system information
- `game` - Launch Pacman game

## Shell Features

- **Command parsing** - Parse command line into arguments
- **Command history** - Keep track of last 32 commands
- **Environment variables** - Store and retrieve system variables
- **NT-like interface** - Familiar Windows-like command prompt
- **Extensible design** - Easy to add new commands

## Usage

The shell can be integrated as a service in the init system:

```c
init_service_t shell_service = {
    .id = 1,
    .name = "Shell",
    .type = SERVICE_SHELL,
    .start = shell_service_start,
    .stop = shell_service_stop,
    .state = 0
};
init_register_service(&init_system, &shell_service);
```

## Future Enhancements

- File system commands (dir, cd, del, copy, type)
- Process management (kill, priority)
- Network commands (ping, ipconfig)
- Scripting support
- Pipe and redirection operators
- Batch file execution
