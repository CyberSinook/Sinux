#pragma once

void printk(const char *fmt, ...);

#define KERN_EMERG   "[EMERG] "
#define KERN_ALERT   "[ALERT] "
#define KERN_CRIT    "[CRIT]  "
#define KERN_ERR     "[ERR]   "
#define KERN_WARNING "[WARN]  "
#define KERN_NOTICE  "[NOTE]  "
#define KERN_INFO    "[INFO]  "
#define KERN_DEBUG   "[DBG]   "
