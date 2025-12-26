#ifndef PCI_H
#define PCI_H

#include <stdint.h>

typedef _Bool bool;
#define true 1
#define false 0

typedef struct pci_device {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t  class_code;
    uint8_t  subclass;
    uint8_t  prog_if;
    uint8_t  revision_id;
    uint32_t bar[6];
    uint8_t  irq_line;
    uint8_t  irq_pin;
    bool     uses_dma;
    volatile uint32_t *mmio_base;
    bool     device_enabled;
    bool     interrupt_pending;
    void*    driver_data;
} pci_device_t;

void pci_init(void);
#endif