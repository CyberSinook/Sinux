#ifndef __NETBSD_DRIVER_TEMPLATE_H__
#define __NETBSD_DRIVER_TEMPLATE_H__

#include <kernel/drivers/netbsd_compat/netbsd_compat.h>

/**
 * NetBSD Driver Template
 * This file provides templates for creating NetBSD-compatible drivers in Sinux
 */

/* Template driver softc structure */
struct template_softc {
    device_t dev;              /* Device pointer */
    resource_t *io;            /* I/O resource */
    resource_t *irq;           /* IRQ resource */
    int io_rid;                /* I/O resource ID */
    int irq_rid;               /* IRQ resource ID */
    void *intr_handler;        /* Interrupt handler cookie */
};

/**
 * Template probe function
 * Called when device is discovered
 */
static int template_probe(device_t dev)
{
    /* Check device signature/ID */
    return 0;  /* Return 0 if probe succeeds */
}

/**
 * Template attach function
 * Called to initialize and start device
 */
static int template_attach(device_t parent, device_t self, void *aux)
{
    struct template_softc *sc = device_get_softc(self);
    
    /* Allocate I/O resources */
    sc->io_rid = 0;
    sc->io = bus_alloc_resource(self, SYS_RES_IOPORT, &sc->io_rid,
                                0, ~0, 1, 0);
    if (!sc->io) {
        return EBUSY;
    }
    
    /* Allocate IRQ resources */
    sc->irq_rid = 0;
    sc->irq = bus_alloc_resource(self, SYS_RES_IRQ, &sc->irq_rid,
                                 0, ~0, 1, 0);
    if (!sc->irq) {
        bus_release_resource(self, SYS_RES_IOPORT, sc->io_rid, sc->io);
        return EBUSY;
    }
    
    /* Setup interrupt handler */
    /* bus_setup_intr(self, sc->irq, 0, template_intr, sc, &sc->intr_handler); */
    
    return 0;  /* Success */
}

/**
 * Template detach function
 * Called when device is removed
 */
static int template_detach(device_t dev, int flags)
{
    struct template_softc *sc = device_get_softc(dev);
    
    /* Teardown interrupt handler */
    if (sc->irq) {
        bus_teardown_intr(dev, sc->irq, sc->intr_handler);
    }
    
    /* Release resources */
    if (sc->io) {
        bus_release_resource(dev, SYS_RES_IOPORT, sc->io_rid, sc->io);
    }
    if (sc->irq) {
        bus_release_resource(dev, SYS_RES_IRQ, sc->irq_rid, sc->irq);
    }
    
    return 0;  /* Success */
}

/**
 * Template shutdown function
 * Called during system shutdown
 */
static int template_shutdown(device_t dev)
{
    return 0;  /* Success */
}

/**
 * Template interrupt handler
 * Called when device generates interrupt
 */
static int template_intr(void *arg)
{
    struct template_softc *sc = (struct template_softc *)arg;
    
    /* Handle interrupt */
    
    return 1;  /* Interrupt was handled */
}

/**
 * Template device switch (cdevsw) entry
 */
static const cdevsw_t template_cdevsw = {
    .name = "template",
    .probe = template_probe,
    .attach = template_attach,
    .detach = template_detach,
    .shutdown = template_shutdown,
};

#endif /* __NETBSD_DRIVER_TEMPLATE_H__ */
