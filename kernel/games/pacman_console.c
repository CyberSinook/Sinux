#include "pacman_console.h"
#include <lib/string.h>

/* Console position tracking */
static int console_cursor_x = 0;
static int console_cursor_y = 0;

/**
 * pacman_console_clear - Clear console
 */
void pacman_console_clear()
{
    /* Print ANSI clear screen command */
    const char *clear_screen = "\033[2J\033[H";
    for (const char *p = clear_screen; *p; p++) {
        outb(0x3F8, *p);
    }
}

/**
 * pacman_console_print_at - Print string at position
 * @x: X position
 * @y: Y position
 * @str: String to print
 */
void pacman_console_print_at(int x, int y, const char *str)
{
    if (!str) return;
    
    /* Send ANSI cursor position command */
    char cmd[32];
    int len = 0;
    cmd[len++] = '\033';
    cmd[len++] = '[';
    
    /* Convert y to string */
    int temp = y + 1;
    int digits = 0;
    int temp_copy = temp;
    while (temp_copy > 0) {
        digits++;
        temp_copy /= 10;
    }
    temp_copy = temp;
    for (int i = digits - 1; i >= 0; i--) {
        cmd[len + i] = (temp % 10) + '0';
        temp /= 10;
    }
    len += digits;
    
    cmd[len++] = ';';
    
    /* Convert x to string */
    temp = x + 1;
    digits = 0;
    temp_copy = temp;
    while (temp_copy > 0) {
        digits++;
        temp_copy /= 10;
    }
    temp_copy = temp;
    for (int i = digits - 1; i >= 0; i--) {
        cmd[len + i] = (temp % 10) + '0';
        temp /= 10;
    }
    len += digits;
    
    cmd[len++] = 'H';
    cmd[len] = 0;
    
    /* Send cursor command */
    for (int i = 0; i < len; i++) {
        outb(0x3F8, cmd[i]);
    }
    
    /* Send string */
    for (int i = 0; str[i]; i++) {
        outb(0x3F8, str[i]);
    }
}

/**
 * pacman_console_render_maze - Render maze
 */
void pacman_console_render_maze(pacman_game_t *game)
{
    if (!game) return;
    
    for (int y = 0; y < PACMAN_GRID_HEIGHT; y++) {
        for (int x = 0; x < PACMAN_GRID_WIDTH; x++) {
            char ch = EMPTY_CHAR;
            uint8_t tile = game->maze[y][x];
            
            switch (tile) {
                case TILE_WALL:
                    ch = WALL_CHAR;
                    break;
                case TILE_PELLET:
                    ch = PELLET_CHAR;
                    break;
                case TILE_POWER_PELLET:
                    ch = POWER_PELLET_CHAR;
                    break;
                default:
                    ch = EMPTY_CHAR;
                    break;
            }
            
            outb(0x3F8, ch);
        }
        outb(0x3F8, '\n');
    }
}

/**
 * pacman_console_render_player - Render pacman
 */
void pacman_console_render_player(pacman_game_t *game)
{
    if (!game) return;
    pacman_console_print_at(game->player.x, game->player.y, "C");
}

/**
 * pacman_console_render_ghosts - Render ghosts
 */
void pacman_console_render_ghosts(pacman_game_t *game)
{
    if (!game) return;
    
    for (int i = 0; i < PACMAN_MAX_GHOSTS; i++) {
        if (game->ghosts[i].state != GHOST_EATEN) {
            char ghost_char = 'G';
            pacman_console_print_at(game->ghosts[i].x, game->ghosts[i].y, "G");
        }
    }
}

/**
 * pacman_console_render_status - Render status bar
 */
void pacman_console_render_status(pacman_game_t *game)
{
    if (!game) return;
    
    /* Simple status output */
    outb(0x3F8, '\n');
    const char *status_fmt = "Score: ";
    for (const char *p = status_fmt; *p; p++) {
        outb(0x3F8, *p);
    }
    
    /* Print score (simplified) */
    int score = game->player.score;
    int digits = 0;
    int temp = score;
    while (temp > 0) {
        digits++;
        temp /= 10;
    }
    
    temp = score;
    for (int i = digits - 1; i >= 0; i--) {
        int digit_val = temp / (1);
        for (int j = 0; j < i; j++) digit_val /= 10;
        outb(0x3F8, (digit_val % 10) + '0');
    }
    
    const char *lives_fmt = " Lives: ";
    for (const char *p = lives_fmt; *p; p++) {
        outb(0x3F8, *p);
    }
    outb(0x3F8, (game->player.lives % 10) + '0');
}

/**
 * pacman_console_render - Full render
 */
void pacman_console_render(pacman_game_t *game)
{
    if (!game) return;
    
    pacman_console_clear();
    pacman_console_render_maze(game);
    pacman_console_render_player(game);
    pacman_console_render_ghosts(game);
    pacman_console_render_status(game);
}
