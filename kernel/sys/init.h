#ifndef __INIT_H__
#define __INIT_H__

#include <stdint.h>

/* Init System Definitions */

/* Init states */
#define INIT_STATE_BOOT         0
#define INIT_STATE_RUNNING      1
#define INIT_STATE_SHUTDOWN     2

/* Service types */
#define SERVICE_GAME            1
#define SERVICE_SHELL           2
#define SERVICE_FILESYSTEM      3
#define SERVICE_NETWORK         4

/* Service structure */
typedef struct {
    int id;                     /* Service ID */
    const char *name;           /* Service name */
    int type;                   /* Service type */
    int (*start)(void);         /* Start function */
    int (*stop)(void);          /* Stop function */
    int state;                  /* Running state */
} init_service_t;

/* Init system structure */
typedef struct {
    int state;                  /* Current state */
    int runlevel;               /* Current runlevel */
    init_service_t *services;   /* List of services */
    int service_count;          /* Number of services */
    int default_service;        /* Default service to run */
} init_system_t;

/* Init functions */
int init_system_init(init_system_t *init);
int init_register_service(init_system_t *init, init_service_t *service);
int init_start_service(init_system_t *init, int service_id);
int init_stop_service(init_system_t *init, int service_id);
int init_run_default_service(init_system_t *init);
int init_set_default_service(init_system_t *init, int service_id);
void init_system_shutdown(init_system_t *init);

#endif /* __INIT_H__ */
