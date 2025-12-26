#include <kernel/arch/x86/pci/pci.h>

bool is_rtl8139(pci_device_t *dev) {
    if (!dev) return false;

    return (dev->vendor_id == 0x10EC && dev->device_id == 0x8139);
}