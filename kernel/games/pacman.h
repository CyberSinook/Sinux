#ifndef __PACMAN_H__
#define __PACMAN_H__

#include <stdint.h>
#include <stddef.h>

/* Pacman Game Definitions */

/* Game constants */
#define PACMAN_GRID_WIDTH   28
#define PACMAN_GRID_HEIGHT  31
#define PACMAN_TILE_SIZE    8

#define PACMAN_MAX_GHOSTS   4
#define PACMAN_PELLET_VALUE 10
#define PACMAN_POWER_PELLET_VALUE 50
#define PACMAN_GHOST_VALUE  200

/* Game states */
#define PACMAN_STATE_RUNNING    1
#define PACMAN_STATE_PAUSED     2
#define PACMAN_STATE_GAME_OVER  3
#define PACMAN_STATE_WIN        4

/* Direction constants */
#define DIR_UP      0
#define DIR_DOWN    1
#define DIR_LEFT    2
#define DIR_RIGHT   3
#define DIR_NONE    4

/* Tile types */
#define TILE_EMPTY       0
#define TILE_WALL        1
#define TILE_PELLET      2
#define TILE_POWER_PELLET 3

/* Ghost states */
#define GHOST_NORMAL    0
#define GHOST_FRIGHTENED 1
#define GHOST_EATEN     2

/* Ghost structure */
typedef struct {
    int x, y;                   /* Position in grid */
    int direction;              /* Current direction */
    int next_direction;         /* Next direction to move */
    int state;                  /* Ghost state (normal/frightened/eaten) */
    int color;                  /* Ghost color */
    int scatter_timer;          /* Scatter mode timer */
    int chase_timer;            /* Chase mode timer */
} pacman_ghost_t;

/* Pacman structure */
typedef struct {
    int x, y;                   /* Position in grid */
    int direction;              /* Current direction */
    int next_direction;         /* Next direction to move */
    int lives;                  /* Remaining lives */
    int score;                  /* Current score */
    int pellets_eaten;          /* Pellets eaten count */
    int power_timer;            /* Power-up timer */
} pacman_player_t;

/* Game structure */
typedef struct {
    uint8_t maze[PACMAN_GRID_HEIGHT][PACMAN_GRID_WIDTH];  /* Maze layout */
    pacman_player_t player;                                 /* Pacman player */
    pacman_ghost_t ghosts[PACMAN_MAX_GHOSTS];              /* Ghosts */
    int state;                                              /* Game state */
    int level;                                              /* Current level */
    int total_pellets;                                      /* Total pellets in maze */
    int frame_count;                                        /* Frame counter for timing */
} pacman_game_t;

/* Game functions */
int pacman_init(pacman_game_t *game);
void pacman_reset_level(pacman_game_t *game);
void pacman_update(pacman_game_t *game);
void pacman_render(pacman_game_t *game);
int pacman_input(pacman_game_t *game, int key);
void pacman_move_pacman(pacman_game_t *game);
void pacman_move_ghost(pacman_game_t *game, int ghost_idx);
int pacman_check_collision(pacman_game_t *game, int x, int y);
int pacman_is_passable(uint8_t tile);
void pacman_generate_maze(pacman_game_t *game);
int pacman_check_win(pacman_game_t *game);
int pacman_check_game_over(pacman_game_t *game);

#endif /* __PACMAN_H__ */
