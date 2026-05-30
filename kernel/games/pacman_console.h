#ifndef __PACMAN_CONSOLE_H__
#define __PACMAN_CONSOLE_H__

#include "pacman.h"
#include <lib/inout.h>

/* Console-based Pacman renderer for text mode */

#define PACMAN_CHAR     'C'
#define GHOST_CHAR      'G'
#define WALL_CHAR       '#'
#define PELLET_CHAR     '.'
#define POWER_PELLET_CHAR '*'
#define EMPTY_CHAR      ' '

/* Console rendering functions */
void pacman_console_clear();
void pacman_console_render(pacman_game_t *game);
void pacman_console_render_maze(pacman_game_t *game);
void pacman_console_render_player(pacman_game_t *game);
void pacman_console_render_ghosts(pacman_game_t *game);
void pacman_console_render_status(pacman_game_t *game);
void pacman_console_print_at(int x, int y, const char *str);

#endif /* __PACMAN_CONSOLE_H__ */
