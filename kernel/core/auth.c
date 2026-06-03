#include "auth.h"
#include "../fs/vfs.h"
#include "../../lib/string.h"
#include "../../lib/printk.h"

#define PASSWD_FILE "/etc/passwd"

static user_entry_t users[MAX_USERS];
static int          user_count = 0;
static user_entry_t *current   = NULL;

static user_entry_t *
find_user(const char *name)
{
    for (int i = 0; i < user_count; i++)
        if (users[i].active && !kstrcmp(users[i].username, name))
            return &users[i];
    return NULL;
}

void auth_save(void)
{
    file_t *f = vfs_open(PASSWD_FILE, O_WRONLY|O_CREAT|O_TRUNC);
    if (!f) return;
    char line[256];
    for (int i = 0; i < user_count; i++) {
        if (!users[i].active) continue;
        int n = 0;
        kstrcpy(line, users[i].username); n = kstrlen(line);
        line[n++] = ':';
        kstrcpy(line+n, users[i].password); n += kstrlen(users[i].password);
        line[n++] = ':';
        char num[12]; int t = 11; num[11]='\0';
        uint32_t v = users[i].uid;
        if (!v) { num[--t]='0'; } else while(v){num[--t]='0'+v%10;v/=10;}
        kstrcpy(line+n, num+t); n += kstrlen(num+t);
        line[n++] = ':';
        kstrcpy(line+n, users[i].home); n += kstrlen(users[i].home);
        line[n++] = ':';
        kstrcpy(line+n, users[i].shell); n += kstrlen(users[i].shell);
        line[n++] = '\n'; line[n] = '\0';
        vfs_write(f, line, n);
    }
    vfs_close(f);
}

void auth_load(void)
{
    file_t *f = vfs_open(PASSWD_FILE, O_RDONLY);
    if (!f) return;
    char buf[4096]; buf[0]='\0';
    int64_t n = vfs_read(f, buf, sizeof(buf)-1);
    vfs_close(f);
    if (n <= 0) return;
    buf[n] = '\0';

    char *line = buf;
    while (*line) {
        char *end = line;
        while (*end && *end != '\n') end++;
        char saved = *end; *end = '\0';

        char *fields[5]; int fc=0;
        char *p = line;
        fields[fc++] = p;
        while (*p && fc < 5) { if (*p==':') { *p='\0'; fields[fc++]=p+1; } p++; }

        if (fc == 5 && user_count < MAX_USERS) {
            user_entry_t *u = &users[user_count++];
            kstrncpy(u->username, fields[0], MAX_USERNAME);
            kstrncpy(u->password, fields[1], MAX_PASSWORD);
            u->uid = 0;
            const char *np = fields[2];
            while (*np >= '0' && *np <= '9') u->uid = u->uid*10 + (*np++)-'0';
            u->gid = u->uid;
            kstrncpy(u->home,  fields[3], MAX_HOMEDIR);
            kstrncpy(u->shell, fields[4], MAX_SHELL);
            u->active = true;
        }

        *end = saved;
        line = (*end) ? end+1 : end;
    }
}

void auth_init(void)
{
    user_count = 0;
    auth_load();

    if (user_count == 0) {
        user_entry_t *root = &users[user_count++];
        kstrcpy(root->username, "root");
        kstrcpy(root->password, "");
        root->uid = 0; root->gid = 0;
        kstrcpy(root->home,  "/root");
        kstrcpy(root->shell, "/bin/sh");
        root->active = true;
        auth_save();
    }

    current = find_user("root");
    if (!current) current = &users[0];

    vfs_create("/root", FT_DIR);
    for (int i = 0; i < user_count; i++) {
        if (!users[i].active) continue;
        vfs_create(users[i].home, FT_DIR);
    }
}

bool auth_check(const char *user, const char *pass)
{
    user_entry_t *u = find_user(user);
    if (!u) return false;
    return kstrcmp(u->password, pass) == 0;
}

bool auth_login(const char *user, const char *pass)
{
    if (!auth_check(user, pass)) return false;
    auth_switch(user);
    return true;
}

void auth_set_password(const char *user, const char *pass)
{
    user_entry_t *u = find_user(user);
    if (!u) return;
    kstrncpy(u->password, pass, MAX_PASSWORD);
    auth_save();
    printk("passwd: password updated for %s\n", user);
}

int auth_add_user(const char *user, const char *pass,
                  uint32_t uid, const char *home)
{
    if (find_user(user)) return -1;
    if (user_count >= MAX_USERS) return -1;
    user_entry_t *u = &users[user_count++];
    kstrncpy(u->username, user, MAX_USERNAME);
    kstrncpy(u->password, pass, MAX_PASSWORD);
    u->uid = uid; u->gid = uid;
    kstrncpy(u->home, home, MAX_HOMEDIR);
    kstrcpy(u->shell, "/bin/sh");
    u->active = true;
    vfs_create(home, FT_DIR);
    auth_save();
    return 0;
}

int auth_del_user(const char *user)
{
    if (!kstrcmp(user, "root")) return -1;
    user_entry_t *u = find_user(user);
    if (!u) return -1;
    u->active = false;
    auth_save();
    return 0;
}

user_entry_t *auth_find_user(const char *user) { return find_user(user); }
const char   *auth_current_user(void)  { return current ? current->username : "root"; }
uint32_t      auth_current_uid(void)   { return current ? current->uid : 0; }
bool          auth_is_root(void)       { return current ? current->uid == 0 : true; }
const char   *auth_current_home(void)  { return current ? current->home : "/root"; }

void auth_switch(const char *username)
{
    user_entry_t *u = find_user(username);
    if (u) current = u;
}
