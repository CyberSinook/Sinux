#include "init.h"
#include <kernel/games/pacman.h>
#include <kernel/games/pacman_console.h>
#include <lib/inout.h>
#include <lib/string.h>
#include <kernel/arch/x86/mm/heap.h>

/* Maximum services */
#define MAX_SERVICES 16

/* Forward declarations for service functions */
static int pacman_service_start(void);
static int pacman_service_stop(void);
static int shell_service_start(void);
static int shell_service_stop(void);

/* Global game instance for pacman service */
static pacman_game_t init_pacman_game;
static int pacman_service_running = 0;

/**
 * pacman_service_start - Start Pacman game service
 * Returns: 0 on success
 */
static int pacman_service_start(void)
{
    const char *msg = "[INIT] Starting Pacman Game Service...\n";
    for (const char *p = msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    /* Initialize game */
    if (pacman_init(&init_pacman_game) != 0) {
        const char *err_msg = "[INIT] Failed to initialize Pacman game\n";
        for (const char *p = err_msg; *p; p++) {
            outb(0x3F8, *p);
        }
        return -1;
    }
    
    pacman_service_running = 1;
    
    const char *start_msg = "[INIT] Pacman Game Service started successfully\n";
    for (const char *p = start_msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    return 0;
}

/**
 * pacman_service_stop - Stop Pacman game service
 * Returns: 0 on success
 */
static int pacman_service_stop(void)
{
    const char *msg = "[INIT] Stopping Pacman Game Service...\n";
    for (const char *p = msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    pacman_service_running = 0;
    
    const char *stop_msg = "[INIT] Pacman Game Service stopped\n";
    for (const char *p = stop_msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    return 0;
}

/**
 * pacman_service_run - Run Pacman game loop (called during service execution)
 */
int pacman_service_run(void)
{
    if (!pacman_service_running) {
        return -1;
    }
    
    uint32_t frame_counter = 0;
    
    /* Print welcome message */
    const char *welcome = "\n=== SINUX PACMAN GAME ===\n";
    for (const char *p = welcome; *p; p++) {
        outb(0x3F8, *p);
    }
    
    const char *controls = "Controls: W=Up, S=Down, A=Left, D=Right, P=Pause, R=Restart\n";
    for (const char *p = controls; *p; p++) {
        outb(0x3F8, *p);
    }
    
    /* Main game loop */
    while (pacman_service_running && init_pacman_game.state != PACMAN_STATE_GAME_OVER && init_pacman_game.state != PACMAN_STATE_WIN) {
        /* Update game state */
        pacman_update(&init_pacman_game);
        
        /* Render game periodically */
        if (frame_counter % 10 == 0) {
            pacman_console_render(&init_pacman_game);
        }
        
        /* Simple frame delay */
        for (uint32_t i = 0; i < 1000000; i++) {
            asm volatile("nop");
        }
        
        frame_counter++;
    }
    
    /* Game end */
    if (init_pacman_game.state == PACMAN_STATE_WIN) {
        const char *win_msg = "\n*** YOU WIN! ***\n";
        for (const char *p = win_msg; *p; p++) {
            outb(0x3F8, *p);
        }
    } else if (init_pacman_game.state == PACMAN_STATE_GAME_OVER) {
        const char *over_msg = "\n*** GAME OVER ***\n";
        for (const char *p = over_msg; *p; p++) {
            outb(0x3F8, *p);
        }
    }
    
    /* Print final score */
    const char *score_msg = "Final Score: ";
    for (const char *p = score_msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    /* Print score digits */
    uint32_t score = init_pacman_game.player.score;
    uint32_t divisor = 10000;
    while (divisor >= 1) {
        uint8_t digit = (score / divisor) % 10;
        outb(0x3F8, '0' + digit);
        divisor /= 10;
    }
    outb(0x3F8, '\n');
    
    pacman_service_running = 0;
    return 0;
}

/**
 * shell_service_start - Start shell service (placeholder)
 * Returns: 0 on success
 */
static int shell_service_start(void)
{
    const char *msg = "[INIT] Starting Shell Service...\n";
    for (const char *p = msg; *p; p++) {
        outb(0x3F8, *p);
    }
    return 0;
}

/**
 * shell_service_stop - Stop shell service
 * Returns: 0 on success
 */
static int shell_service_stop(void)
{
    const char *msg = "[INIT] Stopping Shell Service...\n";
    for (const char *p = msg; *p; p++) {
        outb(0x3F8, *p);
    }
    return 0;
}

/**
 * init_system_init - Initialize init system
 * @init: Pointer to init system
 * Returns: 0 on success, -1 on failure
 */
int init_system_init(init_system_t *init)
{
    if (!init) {
        return -1;
    }
    
    init->state = INIT_STATE_BOOT;
    init->runlevel = 0;
    init->service_count = 0;
    init->services = (init_service_t *)malloc(sizeof(init_service_t) * MAX_SERVICES);
    init->default_service = -1;
    
    if (!init->services) {
        return -1;
    }
    
    const char *init_msg = "[INIT] Initializing Init System...\n";
    for (const char *p = init_msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    return 0;
}

/**
 * init_register_service - Register a service with init
 * @init: Pointer to init system
 * @service: Pointer to service structure
 * Returns: 0 on success, -1 on failure
 */
int init_register_service(init_system_t *init, init_service_t *service)
{
    if (!init || !service || init->service_count >= MAX_SERVICES) {
        return -1;
    }
    
    memcpy(&init->services[init->service_count], service, sizeof(init_service_t));
    init->services[init->service_count].state = 0;
    
    char reg_msg[64] = "[INIT] Registered service: ";
    int len = 0;
    while (reg_msg[len]) len++;
    
    for (int i = 0; service->name[i] && len < 60; i++, len++) {
        reg_msg[len] = service->name[i];
    }
    reg_msg[len++] = '\n';
    reg_msg[len] = 0;
    
    for (const char *p = reg_msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    init->service_count++;
    return 0;
}

/**
 * init_start_service - Start a specific service
 * @init: Pointer to init system
 * @service_id: Service ID
 * Returns: 0 on success, -1 on failure
 */
int init_start_service(init_system_t *init, int service_id)
{
    if (!init || service_id < 0 || service_id >= init->service_count) {
        return -1;
    }
    
    init_service_t *service = &init->services[service_id];
    
    if (service->start) {
        if (service->start() != 0) {
            return -1;
        }
    }
    
    service->state = 1;  /* Running */
    return 0;
}

/**
 * init_stop_service - Stop a specific service
 * @init: Pointer to init system
 * @service_id: Service ID
 * Returns: 0 on success, -1 on failure
 */
int init_stop_service(init_system_t *init, int service_id)
{
    if (!init || service_id < 0 || service_id >= init->service_count) {
        return -1;
    }
    
    init_service_t *service = &init->services[service_id];
    
    if (service->stop) {
        if (service->stop() != 0) {
            return -1;
        }
    }
    
    service->state = 0;  /* Stopped */
    return 0;
}

/**
 * init_set_default_service - Set default service to run
 * @init: Pointer to init system
 * @service_id: Service ID
 * Returns: 0 on success, -1 on failure
 */
int init_set_default_service(init_system_t *init, int service_id)
{
    if (!init || service_id < 0 || service_id >= init->service_count) {
        return -1;
    }
    
    init->default_service = service_id;
    
    const char *msg = "[INIT] Set default service\n";
    for (const char *p = msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    return 0;
}

/**
 * init_run_default_service - Run the default service
 * @init: Pointer to init system
 * Returns: 0 on success, -1 on failure
 */
int init_run_default_service(init_system_t *init)
{
    if (!init || init->default_service < 0 || init->default_service >= init->service_count) {
        const char *err_msg = "[INIT] No default service configured\n";
        for (const char *p = err_msg; *p; p++) {
            outb(0x3F8, *p);
        }
        return -1;
    }
    
    int service_id = init->default_service;
    init_service_t *service = &init->services[service_id];
    
    char run_msg[64] = "[INIT] Running default service: ";
    int len = 0;
    while (run_msg[len]) len++;
    
    for (int i = 0; service->name[i] && len < 60; i++, len++) {
        run_msg[len] = service->name[i];
    }
    run_msg[len++] = '\n';
    run_msg[len] = 0;
    
    for (const char *p = run_msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    if (init_start_service(init, service_id) != 0) {
        return -1;
    }
    
    /* For Pacman game service, run the game loop */
    if (service->type == SERVICE_GAME) {
        pacman_service_run();
    }
    
    return 0;
}

/**
 * init_system_shutdown - Shutdown init system
 * @init: Pointer to init system
 */
void init_system_shutdown(init_system_t *init)
{
    if (!init) return;
    
    const char *msg = "[INIT] Shutting down Init System...\n";
    for (const char *p = msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    /* Stop all services */
    for (int i = 0; i < init->service_count; i++) {
        if (init->services[i].state) {
            init_stop_service(init, i);
        }
    }
    
    init->state = INIT_STATE_SHUTDOWN;
    
    if (init->services) {
        free(init->services);
        init->services = NULL;
    }
    
    const char *shutdown_msg = "[INIT] Init System shutdown complete\n";
    for (const char *p = shutdown_msg; *p; p++) {
        outb(0x3F8, *p);
    }
}
