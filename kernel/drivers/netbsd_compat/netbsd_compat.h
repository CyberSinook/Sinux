#ifndef __NETBSD_COMPAT_H__
#define __NETBSD_COMPAT_H__

#include <stdint.h>
#include <stddef.h>

/* NetBSD Compatibility Layer
 * Provides NetBSD-compatible interfaces for driver development
 */

/* NetBSD Device Types */
typedef int device_t;
typedef int devclass_t;

/* Device flags */
#define DVF_ACTIVE      0x0001
#define DVF_DETACHING   0x0002
#define DVF_QUIESCE     0x0004

/* Device probe results */
#define ENXIO           6     /* Device not configured */
#define ENODEV          19    /* Operation not supported by device */
#define EBUSY           16    /* Resource busy */

/* NetBSD Device Structure */
struct device {
    const char *name;              /* Device name */
    device_t parent;               /* Parent device */
    void *softc;                   /* Driver-specific data */
    int unit;                      /* Device unit number */
    int flags;                     /* Device flags */
    devclass_t devclass;           /* Device class */
    void (*detach)(device_t);      /* Detach function pointer */
};

/* NetBSD Device Driver Operations */
typedef struct {
    const char *name;                                           /* Driver name */
    int (*probe)(device_t parent);                             /* Probe function */
    int (*attach)(device_t parent, device_t self, void *aux); /* Attach function */
    int (*detach)(device_t self, int flags);                  /* Detach function */
    int (*shutdown)(device_t self);                           /* Shutdown function */
} cdevsw_t;  /* Character device switch */

typedef struct {
    const char *name;                                           /* Driver name */
    int (*probe)(device_t parent);                             /* Probe function */
    int (*attach)(device_t parent, device_t self, void *aux); /* Attach function */
    int (*detach)(device_t self, int flags);                  /* Detach function */
    int (*shutdown)(device_t self);                           /* Shutdown function */
} bdevsw_t;  /* Block device switch */

/* Interrupt Handler Type */
typedef int (*intr_handler_t)(void *arg);

/* NetBSD Style Device Properties */
typedef struct {
    const char *name;
    int type;
    union {
        uint32_t integer;
        const char *string;
        void *pointer;
    } value;
} device_property_t;

/* Property types */
#define PROP_TYPE_INT       1
#define PROP_TYPE_STRING    2
#define PROP_TYPE_POINTER   3

/* NetBSD Event Handler Queue */
typedef struct event_handler_node {
    void (*func)(void *);
    void *arg;
    struct event_handler_node *next;
} event_handler_node_t;

typedef struct {
    const char *name;
    event_handler_node_t *handlers;
} event_handler_list_t;

/* NetBSD Autoconfiguration Functions */
device_t device_add_child(device_t parent, const char *name, int unit);
int device_probe_and_attach(device_t dev);
int device_detach(device_t dev, int flags);
int device_shutdown(device_t dev);
int device_is_attached(device_t dev);

/* NetBSD Device Property Functions */
int device_get_property(device_t dev, const char *prop, device_property_t *val);
int device_set_property(device_t dev, const char *prop, device_property_t *val);
int device_get_unit(device_t dev);
const char *device_get_name(device_t dev);
device_t device_get_parent(device_t dev);
void *device_get_softc(device_t dev);

/* NetBSD Resource Management */
typedef struct resource {
    int type;              /* Resource type (SYS_RES_*) */
    int rid;               /* Resource ID */
    uint32_t start;        /* Start address/number */
    uint32_t size;         /* Size */
    void *r_bustag;        /* Bus tag */
    void *r_bushandle;     /* Bus handle */
} resource_t;

/* Resource types */
#define SYS_RES_IRQ         1
#define SYS_RES_DRQ         2
#define SYS_RES_IOPORT      3
#define SYS_RES_MEMORY      4

resource_t *bus_alloc_resource(device_t dev, int type, int *rid, 
                                uint32_t start, uint32_t end, uint32_t count, int flags);
int bus_release_resource(device_t dev, int type, int rid, resource_t *res);
int bus_setup_intr(device_t dev, resource_t *r, int flags, 
                   intr_handler_t handler, void *arg, void **cookiep);
int bus_teardown_intr(device_t dev, resource_t *r, void *cookie);

/* NetBSD Memory I/O Functions */
uint8_t bus_read_1(resource_t *res, uint32_t offset);
uint16_t bus_read_2(resource_t *res, uint32_t offset);
uint32_t bus_read_4(resource_t *res, uint32_t offset);
void bus_write_1(resource_t *res, uint32_t offset, uint8_t value);
void bus_write_2(resource_t *res, uint32_t offset, uint16_t value);
void bus_write_4(resource_t *res, uint32_t offset, uint32_t value);
void bus_read_multi_1(resource_t *res, uint32_t offset, uint8_t *dst, size_t count);
void bus_write_multi_1(resource_t *res, uint32_t offset, const uint8_t *src, size_t count);

/* NetBSD Event Handler Functions */
int eventhandler_register(event_handler_list_t *list, const char *name, 
                         void (*func)(void *), void *arg);
int eventhandler_deregister(event_handler_list_t *list, const char *name, 
                            void (*func)(void *));
void eventhandler_invoke(event_handler_list_t *list, void *arg);

/* Device Class Registration */
int devclass_create(const char *classname, devclass_t *dcp);
device_t devclass_get_device(devclass_t dc, int unit);
int devclass_get_maxunit(devclass_t dc);

/* Driver Registration */
int driver_module(const char *name, device_t bus, cdevsw_t *devsw, 
                 devclass_t *devclassp, void *evh, void *arg);
int driver_unmodule(const char *name);

#endif /* __NETBSD_COMPAT_H__ */
