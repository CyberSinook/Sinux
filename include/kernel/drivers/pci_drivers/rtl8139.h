#ifndef RTL8139_H
#define RTL8139_H

#include <kernel/arch/x86/pci/pci.h>

bool is_rtl8139(pci_device_t *dev);

#endif