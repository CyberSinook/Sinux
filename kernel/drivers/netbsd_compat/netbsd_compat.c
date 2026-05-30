#include "netbsd_compat.h"
#include <lib/string.h>
#include <kernel/arch/x86/mm/heap.h>

/* NetBSD Device Database */
#define MAX_DEVICES 128
#define MAX_DEVCLASSES 32
#define MAX_RESOURCES 256

static struct device device_db[MAX_DEVICES];
static int device_count = 0;

static struct {
    const char *name;
    int unit_count;
} devclass_db[MAX_DEVCLASSES];
static int devclass_count = 0;

static resource_t resource_db[MAX_RESOURCES];
static int resource_count = 0;

static event_handler_list_t event_handlers[16];

/**
 * device_add_child - Add a child device to a parent
 * @parent: Parent device
 * @name: Device name
 * @unit: Device unit number
 * Returns: Pointer to new device, NULL on failure
 */
device_t device_add_child(device_t parent, const char *name, int unit)
{
    if (device_count >= MAX_DEVICES || !name) {
        return NULL;
    }
    
    struct device *dev = &device_db[device_count];
    dev->name = name;
    dev->parent = parent;
    dev->unit = unit;
    dev->flags = 0;
    dev->softc = NULL;
    
    return (device_t)device_count++;
}

/**
 * device_probe_and_attach - Probe and attach a device
 * @dev: Device to probe and attach
 * Returns: 0 on success, error code on failure
 */
int device_probe_and_attach(device_t dev)
{
    if (dev < 0 || dev >= device_count) {
        return ENXIO;
    }
    
    struct device *d = &device_db[dev];
    d->flags |= DVF_ACTIVE;
    
    return 0;
}

/**
 * device_detach - Detach a device
 * @dev: Device to detach
 * @flags: Detach flags
 * Returns: 0 on success, error code on failure
 */
int device_detach(device_t dev, int flags)
{
    if (dev < 0 || dev >= device_count) {
        return ENXIO;
    }
    
    struct device *d = &device_db[dev];
    d->flags &= ~DVF_ACTIVE;
    d->flags |= DVF_DETACHING;
    
    if (d->detach) {
        d->detach(dev);
    }
    
    return 0;
}

/**
 * device_shutdown - Shutdown a device
 * @dev: Device to shutdown
 * Returns: 0 on success
 */
int device_shutdown(device_t dev)
{
    if (dev < 0 || dev >= device_count) {
        return ENXIO;
    }
    
    return 0;
}

/**
 * device_is_attached - Check if device is attached
 * @dev: Device to check
 * Returns: 1 if attached, 0 otherwise
 */
int device_is_attached(device_t dev)
{
    if (dev < 0 || dev >= device_count) {
        return 0;
    }
    
    return (device_db[dev].flags & DVF_ACTIVE) ? 1 : 0;
}

/**
 * device_get_unit - Get device unit number
 * @dev: Device
 * Returns: Unit number
 */
int device_get_unit(device_t dev)
{
    if (dev < 0 || dev >= device_count) {
        return -1;
    }
    
    return device_db[dev].unit;
}

/**
 * device_get_name - Get device name
 * @dev: Device
 * Returns: Device name
 */
const char *device_get_name(device_t dev)
{
    if (dev < 0 || dev >= device_count) {
        return NULL;
    }
    
    return device_db[dev].name;
}

/**
 * device_get_parent - Get parent device
 * @dev: Device
 * Returns: Parent device or NULL
 */
device_t device_get_parent(device_t dev)
{
    if (dev < 0 || dev >= device_count) {
        return NULL;
    }
    
    return device_db[dev].parent;
}

/**
 * device_get_softc - Get driver-specific data
 * @dev: Device
 * Returns: Pointer to softc or NULL
 */
void *device_get_softc(device_t dev)
{
    if (dev < 0 || dev >= device_count) {
        return NULL;
    }
    
    return device_db[dev].softc;
}

/**
 * device_get_property - Get device property
 * @dev: Device
 * @prop: Property name
 * @val: Pointer to value structure
 * Returns: 0 on success, -1 if not found
 */
int device_get_property(device_t dev, const char *prop, device_property_t *val)
{
    if (dev < 0 || dev >= device_count || !prop || !val) {
        return -1;
    }
    
    /* Placeholder - in real implementation would search property database */
    return -1;
}

/**
 * device_set_property - Set device property
 * @dev: Device
 * @prop: Property name
 * @val: Pointer to value structure
 * Returns: 0 on success
 */
int device_set_property(device_t dev, const char *prop, device_property_t *val)
{
    if (dev < 0 || dev >= device_count || !prop || !val) {
        return -1;
    }
    
    /* Placeholder - in real implementation would store in property database */
    return 0;
}

/**
 * bus_alloc_resource - Allocate a resource
 * @dev: Device
 * @type: Resource type
 * @rid: Resource ID pointer
 * @start: Start address/number
 * @end: End address/number
 * @count: Count
 * @flags: Allocation flags
 * Returns: Pointer to resource or NULL on failure
 */
resource_t *bus_alloc_resource(device_t dev, int type, int *rid,
                                uint32_t start, uint32_t end, uint32_t count, int flags)
{
    if (resource_count >= MAX_RESOURCES) {
        return NULL;
    }
    
    resource_t *res = &resource_db[resource_count++];
    res->type = type;
    res->rid = rid ? *rid : 0;
    res->start = start;
    res->size = (end - start + 1);
    res->r_bustag = NULL;
    res->r_bushandle = NULL;
    
    if (rid) {
        *rid = res->rid;
    }
    
    return res;
}

/**
 * bus_release_resource - Release a resource
 * @dev: Device
 * @type: Resource type
 * @rid: Resource ID
 * @res: Pointer to resource
 * Returns: 0 on success
 */
int bus_release_resource(device_t dev, int type, int rid, resource_t *res)
{
    /* Mark resource as available for reuse */
    if (res) {
        res->type = 0;
    }
    return 0;
}

/**
 * bus_setup_intr - Setup interrupt handler
 * @dev: Device
 * @r: Resource
 * @flags: Setup flags
 * @handler: Interrupt handler function
 * @arg: Handler argument
 * @cookiep: Pointer to store cookie for teardown
 * Returns: 0 on success
 */
int bus_setup_intr(device_t dev, resource_t *r, int flags,
                   intr_handler_t handler, void *arg, void **cookiep)
{
    if (!r || !handler || r->type != SYS_RES_IRQ) {
        return EBUSY;
    }
    
    /* Placeholder - would register with interrupt controller */
    if (cookiep) {
        *cookiep = (void *)handler;
    }
    
    return 0;
}

/**
 * bus_teardown_intr - Teardown interrupt handler
 * @dev: Device
 * @r: Resource
 * @cookie: Cookie from setup
 * Returns: 0 on success
 */
int bus_teardown_intr(device_t dev, resource_t *r, void *cookie)
{
    /* Placeholder - would deregister from interrupt controller */
    return 0;
}

/**
 * bus_read_1 - Read 8-bit value from device memory
 */
uint8_t bus_read_1(resource_t *res, uint32_t offset)
{
    if (!res) return 0;
    uint32_t addr = res->start + offset;
    return *(volatile uint8_t *)addr;
}

/**
 * bus_read_2 - Read 16-bit value from device memory
 */
uint16_t bus_read_2(resource_t *res, uint32_t offset)
{
    if (!res) return 0;
    uint32_t addr = res->start + offset;
    return *(volatile uint16_t *)addr;
}

/**
 * bus_read_4 - Read 32-bit value from device memory
 */
uint32_t bus_read_4(resource_t *res, uint32_t offset)
{
    if (!res) return 0;
    uint32_t addr = res->start + offset;
    return *(volatile uint32_t *)addr;
}

/**
 * bus_write_1 - Write 8-bit value to device memory
 */
void bus_write_1(resource_t *res, uint32_t offset, uint8_t value)
{
    if (!res) return;
    uint32_t addr = res->start + offset;
    *(volatile uint8_t *)addr = value;
}

/**
 * bus_write_2 - Write 16-bit value to device memory
 */
void bus_write_2(resource_t *res, uint32_t offset, uint16_t value)
{
    if (!res) return;
    uint32_t addr = res->start + offset;
    *(volatile uint16_t *)addr = value;
}

/**
 * bus_write_4 - Write 32-bit value to device memory
 */
void bus_write_4(resource_t *res, uint32_t offset, uint32_t value)
{
    if (!res) return;
    uint32_t addr = res->start + offset;
    *(volatile uint32_t *)addr = value;
}

/**
 * bus_read_multi_1 - Read multiple 8-bit values
 */
void bus_read_multi_1(resource_t *res, uint32_t offset, uint8_t *dst, size_t count)
{
    if (!res || !dst) return;
    uint32_t addr = res->start + offset;
    for (size_t i = 0; i < count; i++) {
        dst[i] = *(volatile uint8_t *)(addr + i);
    }
}

/**
 * bus_write_multi_1 - Write multiple 8-bit values
 */
void bus_write_multi_1(resource_t *res, uint32_t offset, const uint8_t *src, size_t count)
{
    if (!res || !src) return;
    uint32_t addr = res->start + offset;
    for (size_t i = 0; i < count; i++) {
        *(volatile uint8_t *)(addr + i) = src[i];
    }
}

/**
 * eventhandler_register - Register event handler
 */
int eventhandler_register(event_handler_list_t *list, const char *name,
                         void (*func)(void *), void *arg)
{
    if (!list || !func) {
        return -1;
    }
    
    event_handler_node_t *node = (event_handler_node_t *)malloc(sizeof(event_handler_node_t));
    if (!node) {
        return -1;
    }
    
    node->func = func;
    node->arg = arg;
    node->next = list->handlers;
    list->handlers = node;
    
    return 0;
}

/**
 * eventhandler_deregister - Deregister event handler
 */
int eventhandler_deregister(event_handler_list_t *list, const char *name,
                            void (*func)(void *))
{
    if (!list || !func) {
        return -1;
    }
    
    event_handler_node_t **pp = &list->handlers;
    while (*pp) {
        if ((*pp)->func == func) {
            event_handler_node_t *tmp = *pp;
            *pp = (*pp)->next;
            free(tmp);
            return 0;
        }
        pp = &(*pp)->next;
    }
    
    return -1;
}

/**
 * eventhandler_invoke - Invoke event handlers
 */
void eventhandler_invoke(event_handler_list_t *list, void *arg)
{
    if (!list) return;
    
    event_handler_node_t *node = list->handlers;
    while (node) {
        if (node->func) {
            node->func(arg);
        }
        node = node->next;
    }
}

/**
 * devclass_create - Create a device class
 */
int devclass_create(const char *classname, devclass_t *dcp)
{
    if (devclass_count >= MAX_DEVCLASSES || !classname) {
        return -1;
    }
    
    devclass_db[devclass_count].name = classname;
    devclass_db[devclass_count].unit_count = 0;
    
    if (dcp) {
        *dcp = devclass_count;
    }
    
    return devclass_count++;
}

/**
 * devclass_get_device - Get device from device class
 */
device_t devclass_get_device(devclass_t dc, int unit)
{
    if (dc < 0 || dc >= devclass_count) {
        return NULL;
    }
    
    for (int i = 0; i < device_count; i++) {
        if (device_db[i].devclass == dc && device_db[i].unit == unit) {
            return (device_t)i;
        }
    }
    
    return NULL;
}

/**
 * devclass_get_maxunit - Get max unit in device class
 */
int devclass_get_maxunit(devclass_t dc)
{
    if (dc < 0 || dc >= devclass_count) {
        return -1;
    }
    
    int maxunit = -1;
    for (int i = 0; i < device_count; i++) {
        if (device_db[i].devclass == dc && device_db[i].unit > maxunit) {
            maxunit = device_db[i].unit;
        }
    }
    
    return maxunit;
}

/**
 * driver_module - Register driver module
 */
int driver_module(const char *name, device_t bus, cdevsw_t *devsw,
                 devclass_t *devclassp, void *evh, void *arg)
{
    if (!name || !devsw) {
        return -1;
    }
    
    devclass_t dc;
    if (devclass_create(name, &dc) != 0) {
        return -1;
    }
    
    if (devclassp) {
        *devclassp = dc;
    }
    
    return 0;
}

/**
 * driver_unmodule - Unregister driver module
 */
int driver_unmodule(const char *name)
{
    if (!name) {
        return -1;
    }
    
    return 0;
}
