#pragma once
#include <stdbool.h>
#include <stdint.h>

#define MAX_USERS     64
#define MAX_USERNAME  32
#define MAX_PASSWORD  128
#define MAX_HOMEDIR   128
#define MAX_SHELL     64

typedef struct {
    char     username[MAX_USERNAME];
    char     password[MAX_PASSWORD];
    uint32_t uid;
    uint32_t gid;
    char     home[MAX_HOMEDIR];
    char     shell[MAX_SHELL];
    bool     active;
} user_entry_t;

void        auth_init(void);
bool        auth_login(const char *user, const char *pass);
bool        auth_check(const char *user, const char *pass);
void        auth_set_password(const char *user, const char *pass);

int         auth_add_user(const char *user, const char *pass,
                          uint32_t uid, const char *home);
int         auth_del_user(const char *user);
user_entry_t *auth_find_user(const char *user);

const char *auth_current_user(void);
uint32_t    auth_current_uid(void);
bool        auth_is_root(void);
const char *auth_current_home(void);
void        auth_switch(const char *username);

void        auth_save(void);
void        auth_load(void);
