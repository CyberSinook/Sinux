#include "pacman.h"
#include <lib/string.h>
#include <kernel/arch/x86/mm/heap.h>

/* Simple maze layout - 1 = wall, 0 = empty, 2 = pellet, 3 = power pellet */
static const uint8_t default_maze[PACMAN_GRID_HEIGHT][PACMAN_GRID_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,2,1,1,1,2,1},
    {1,3,1,1,2,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,2,1,1,3,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,2,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,2,1,1,1,1,1},
    {1,1,1,1,2,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,2,1,1,1,1,1},
    {1,1,1,1,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1},
    {1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,1,1,1,1,1},
    {1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,2,1,1,1,1,1},
    {0,0,0,0,2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,2,0,0,0,0,0},
    {0,0,0,0,2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,2,0,0,0,0,0},
    {1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,2,1,1,1,1,1},
    {1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,1,1,1,1,1},
    {1,1,1,1,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1},
    {1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,2,1,1,1,2,1},
    {1,3,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,3,1},
    {1,1,2,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,2,2,2,2,1,1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

/**
 * pacman_init - Initialize Pacman game
 * @game: Pointer to game structure
 * Returns: 0 on success, -1 on failure
 */
int pacman_init(pacman_game_t *game)
{
    if (!game) {
        return -1;
    }
    
    /* Copy default maze */
    memcpy(game->maze, default_maze, sizeof(default_maze));
    
    /* Initialize player */
    game->player.x = 13;
    game->player.y = 23;
    game->player.direction = DIR_NONE;
    game->player.next_direction = DIR_NONE;
    game->player.lives = 3;
    game->player.score = 0;
    game->player.pellets_eaten = 0;
    game->player.power_timer = 0;
    
    /* Initialize ghosts */
    /* Blinky (red) */
    game->ghosts[0].x = 13;
    game->ghosts[0].y = 11;
    game->ghosts[0].direction = DIR_LEFT;
    game->ghosts[0].state = GHOST_NORMAL;
    game->ghosts[0].color = 1;  /* Red */
    
    /* Pinky (pink) */
    game->ghosts[1].x = 12;
    game->ghosts[1].y = 14;
    game->ghosts[1].direction = DIR_UP;
    game->ghosts[1].state = GHOST_NORMAL;
    game->ghosts[1].color = 2;  /* Pink */
    
    /* Inky (cyan) */
    game->ghosts[2].x = 14;
    game->ghosts[2].y = 14;
    game->ghosts[2].direction = DIR_DOWN;
    game->ghosts[2].state = GHOST_NORMAL;
    game->ghosts[2].color = 3;  /* Cyan */
    
    /* Clyde (orange) */
    game->ghosts[3].x = 13;
    game->ghosts[3].y = 13;
    game->ghosts[3].direction = DIR_RIGHT;
    game->ghosts[3].state = GHOST_NORMAL;
    game->ghosts[3].color = 4;  /* Orange */
    
    /* Count total pellets */
    game->total_pellets = 0;
    for (int y = 0; y < PACMAN_GRID_HEIGHT; y++) {
        for (int x = 0; x < PACMAN_GRID_WIDTH; x++) {
            if (game->maze[y][x] == TILE_PELLET || game->maze[y][x] == TILE_POWER_PELLET) {
                game->total_pellets++;
            }
        }
    }
    
    game->state = PACMAN_STATE_RUNNING;
    game->level = 1;
    game->frame_count = 0;
    
    return 0;
}

/**
 * pacman_is_passable - Check if a tile can be passed
 * @tile: Tile type
 * Returns: 1 if passable, 0 if wall
 */
int pacman_is_passable(uint8_t tile)
{
    return tile != TILE_WALL;
}

/**
 * pacman_check_collision - Check if position has a collision
 * @game: Game pointer
 * @x, @y: Position to check
 * Returns: Tile type at position
 */
int pacman_check_collision(pacman_game_t *game, int x, int y)
{
    if (x < 0 || x >= PACMAN_GRID_WIDTH || y < 0 || y >= PACMAN_GRID_HEIGHT) {
        return TILE_WALL;
    }
    return game->maze[y][x];
}

/**
 * pacman_move_pacman - Move pacman based on current direction
 * @game: Game pointer
 */
void pacman_move_pacman(pacman_game_t *game)
{
    int new_x = game->player.x;
    int new_y = game->player.y;
    int direction = game->player.direction;
    
    /* Try next direction if current blocked */
    if (game->player.next_direction != DIR_NONE) {
        int next_x = game->player.x;
        int next_y = game->player.y;
        
        switch (game->player.next_direction) {
            case DIR_UP:    next_y--; break;
            case DIR_DOWN:  next_y++; break;
            case DIR_LEFT:  next_x--; break;
            case DIR_RIGHT: next_x++; break;
        }
        
        if (pacman_is_passable(pacman_check_collision(game, next_x, next_y))) {
            game->player.direction = game->player.next_direction;
            game->player.next_direction = DIR_NONE;
            new_x = next_x;
            new_y = next_y;
        }
    }
    
    /* Move in current direction */
    if (direction != DIR_NONE) {
        switch (direction) {
            case DIR_UP:
                if (new_y > 0 && pacman_is_passable(pacman_check_collision(game, new_x, new_y - 1))) {
                    new_y--;
                }
                break;
            case DIR_DOWN:
                if (new_y < PACMAN_GRID_HEIGHT - 1 && pacman_is_passable(pacman_check_collision(game, new_x, new_y + 1))) {
                    new_y++;
                }
                break;
            case DIR_LEFT:
                if (new_x > 0 && pacman_is_passable(pacman_check_collision(game, new_x - 1, new_y))) {
                    new_x--;
                }
                break;
            case DIR_RIGHT:
                if (new_x < PACMAN_GRID_WIDTH - 1 && pacman_is_passable(pacman_check_collision(game, new_x + 1, new_y))) {
                    new_x++;
                }
                break;
        }
    }
    
    game->player.x = new_x;
    game->player.y = new_y;
    
    /* Eat pellet */
    uint8_t tile = game->maze[new_y][new_x];
    if (tile == TILE_PELLET) {
        game->maze[new_y][new_x] = TILE_EMPTY;
        game->player.score += PACMAN_PELLET_VALUE;
        game->player.pellets_eaten++;
    } else if (tile == TILE_POWER_PELLET) {
        game->maze[new_y][new_x] = TILE_EMPTY;
        game->player.score += PACMAN_POWER_PELLET_VALUE;
        game->player.power_timer = 300;  /* 5 seconds at 60 FPS */
        
        /* Turn ghosts frightened */
        for (int i = 0; i < PACMAN_MAX_GHOSTS; i++) {
            if (game->ghosts[i].state == GHOST_NORMAL) {
                game->ghosts[i].state = GHOST_FRIGHTENED;
            }
        }
    }
}

/**
 * pacman_move_ghost - Move a ghost using simple AI
 * @game: Game pointer
 * @ghost_idx: Ghost index
 */
void pacman_move_ghost(pacman_game_t *game, int ghost_idx)
{
    if (ghost_idx < 0 || ghost_idx >= PACMAN_MAX_GHOSTS) {
        return;
    }
    
    pacman_ghost_t *ghost = &game->ghosts[ghost_idx];
    int new_x = ghost->x;
    int new_y = ghost->y;
    
    /* Simple AI: Move towards pacman if normal, random if frightened */
    if (ghost->state == GHOST_NORMAL) {
        /* Simple chase AI */
        if (ghost->x < game->player.x && pacman_is_passable(pacman_check_collision(game, ghost->x + 1, ghost->y))) {
            new_x = ghost->x + 1;
        } else if (ghost->x > game->player.x && pacman_is_passable(pacman_check_collision(game, ghost->x - 1, ghost->y))) {
            new_x = ghost->x - 1;
        } else if (ghost->y < game->player.y && pacman_is_passable(pacman_check_collision(game, ghost->x, ghost->y + 1))) {
            new_y = ghost->y + 1;
        } else if (ghost->y > game->player.y && pacman_is_passable(pacman_check_collision(game, ghost->x, ghost->y - 1))) {
            new_y = ghost->y - 1;
        }
    } else if (ghost->state == GHOST_FRIGHTENED) {
        /* Random movement */
        int rand_dir = (ghost_idx + game->frame_count) % 4;
        switch (rand_dir) {
            case 0:  /* Up */
                if (pacman_is_passable(pacman_check_collision(game, ghost->x, ghost->y - 1))) {
                    new_y = ghost->y - 1;
                }
                break;
            case 1:  /* Down */
                if (pacman_is_passable(pacman_check_collision(game, ghost->x, ghost->y + 1))) {
                    new_y = ghost->y + 1;
                }
                break;
            case 2:  /* Left */
                if (pacman_is_passable(pacman_check_collision(game, ghost->x - 1, ghost->y))) {
                    new_x = ghost->x - 1;
                }
                break;
            case 3:  /* Right */
                if (pacman_is_passable(pacman_check_collision(game, ghost->x + 1, ghost->y))) {
                    new_x = ghost->x + 1;
                }
                break;
        }
    }
    
    ghost->x = new_x;
    ghost->y = new_y;
}

/**
 * pacman_update - Update game state
 * @game: Game pointer
 */
void pacman_update(pacman_game_t *game)
{
    if (!game || game->state == PACMAN_STATE_PAUSED) {
        return;
    }
    
    game->frame_count++;
    
    /* Move pacman every 2 frames */
    if (game->frame_count % 2 == 0) {
        pacman_move_pacman(game);
    }
    
    /* Move ghosts every 3 frames */
    if (game->frame_count % 3 == 0) {
        for (int i = 0; i < PACMAN_MAX_GHOSTS; i++) {
            pacman_move_ghost(game, i);
        }
    }
    
    /* Update power-up timer */
    if (game->player.power_timer > 0) {
        game->player.power_timer--;
        if (game->player.power_timer == 0) {
            /* Ghosts return to normal */
            for (int i = 0; i < PACMAN_MAX_GHOSTS; i++) {
                if (game->ghosts[i].state == GHOST_FRIGHTENED) {
                    game->ghosts[i].state = GHOST_NORMAL;
                }
            }
        }
    }
    
    /* Check collisions with ghosts */
    for (int i = 0; i < PACMAN_MAX_GHOSTS; i++) {
        if (game->ghosts[i].x == game->player.x && game->ghosts[i].y == game->player.y) {
            if (game->ghosts[i].state == GHOST_FRIGHTENED) {
                game->player.score += PACMAN_GHOST_VALUE;
                game->ghosts[i].state = GHOST_EATEN;
            } else if (game->ghosts[i].state == GHOST_NORMAL) {
                game->player.lives--;
                if (game->player.lives == 0) {
                    game->state = PACMAN_STATE_GAME_OVER;
                } else {
                    /* Reset positions */
                    pacman_reset_level(game);
                }
            }
        }
    }
    
    /* Check win condition */
    if (pacman_check_win(game)) {
        game->state = PACMAN_STATE_WIN;
    }
}

/**
 * pacman_input - Handle player input
 * @game: Game pointer
 * @key: Key code
 * Returns: 0 if handled, -1 otherwise
 */
int pacman_input(pacman_game_t *game, int key)
{
    if (!game) {
        return -1;
    }
    
    switch (key) {
        case 'w': case 'W':  /* Up */
            game->player.next_direction = DIR_UP;
            break;
        case 's': case 'S':  /* Down */
            game->player.next_direction = DIR_DOWN;
            break;
        case 'a': case 'A':  /* Left */
            game->player.next_direction = DIR_LEFT;
            break;
        case 'd': case 'D':  /* Right */
            game->player.next_direction = DIR_RIGHT;
            break;
        case 'p': case 'P':  /* Pause */
            game->state = (game->state == PACMAN_STATE_PAUSED) ? PACMAN_STATE_RUNNING : PACMAN_STATE_PAUSED;
            break;
        case 'r': case 'R':  /* Restart */
            pacman_init(game);
            break;
        default:
            return -1;
    }
    
    return 0;
}

/**
 * pacman_reset_level - Reset level after death
 * @game: Game pointer
 */
void pacman_reset_level(pacman_game_t *game)
{
    if (!game) return;
    
    game->player.x = 13;
    game->player.y = 23;
    game->player.direction = DIR_NONE;
    game->player.next_direction = DIR_NONE;
    
    /* Reset ghosts */
    game->ghosts[0].x = 13;
    game->ghosts[0].y = 11;
    game->ghosts[1].x = 12;
    game->ghosts[1].y = 14;
    game->ghosts[2].x = 14;
    game->ghosts[2].y = 14;
    game->ghosts[3].x = 13;
    game->ghosts[3].y = 13;
}

/**
 * pacman_check_win - Check if player won
 * @game: Game pointer
 * Returns: 1 if won, 0 otherwise
 */
int pacman_check_win(pacman_game_t *game)
{
    if (!game) return 0;
    return game->player.pellets_eaten >= game->total_pellets;
}

/**
 * pacman_check_game_over - Check if game is over
 * @game: Game pointer
 * Returns: 1 if game over, 0 otherwise
 */
int pacman_check_game_over(pacman_game_t *game)
{
    if (!game) return 0;
    return game->state == PACMAN_STATE_GAME_OVER;
}

/**
 * pacman_render - Render game (placeholder for video driver integration)
 * @game: Game pointer
 */
void pacman_render(pacman_game_t *game)
{
    if (!game) return;
    /* Rendering would be handled by video driver */
    /* This is a placeholder for integration with video.c */
}
