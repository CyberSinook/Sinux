#include <stdint.h>
#include <kernel/arch/x86/pci/pci.h>
#include <lib/inout.h>
#include <kernel/drivers/pci_drivers/rtl8139.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

pci_device_t pci_devices[256];
int pci_device_count = 0;

uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address =
        (1U << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)func << 8) |
        (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_scan_function(uint8_t bus, uint8_t device, uint8_t func) {
    uint32_t vendor_device = pci_config_read(bus, device, func, 0x00);
    uint16_t vendor_id = vendor_device & 0xFFFF;
    if (vendor_id == 0xFFFF) return;

    uint16_t device_id = (vendor_device >> 16) & 0xFFFF;

    uint32_t class_reg = pci_config_read(bus, device, func, 0x08);
    uint8_t revision_id = class_reg & 0xFF;
    uint8_t prog_if     = (class_reg >> 8) & 0xFF;
    uint8_t subclass    = (class_reg >> 16) & 0xFF;
    uint8_t class_code  = (class_reg >> 24) & 0xFF;

    pci_device_t *dev = &pci_devices[pci_device_count++];
    dev->vendor_id = vendor_id;
    dev->device_id = device_id;
    dev->class_code = class_code;
    dev->subclass = subclass;
    dev->prog_if = prog_if;
    dev->revision_id = revision_id;

    for (int i = 0; i < 6; i++) {
        dev->bar[i] = pci_config_read(bus, device, func, 0x10 + i*4);
    }

    uint32_t irq_reg = pci_config_read(bus, device, func, 0x3C);
    dev->irq_line = irq_reg & 0xFF;
    dev->irq_pin  = (irq_reg >> 8) & 0xFF;

    dev->uses_dma = false;
    dev->mmio_base = 0;
    dev->device_enabled = true;
    dev->interrupt_pending = false;
    dev->driver_data = 0;
}

void pci_scan_device(uint8_t bus, uint8_t device) {
    uint32_t header = pci_config_read(bus, device, 0, 0x0C);
    if (header & 0x00800000) { // multi-function device
        for (int func = 0; func < 8; func++) {
            pci_scan_function(bus, device, func);
        }
    } else {
        pci_scan_function(bus, device, 0);
    }
}

void pci_scan_bus() {
    pci_device_count = 0;
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            pci_scan_device(bus, device);
        }
    }
}

bool (*pci_drivers[])(pci_device_t*) = {is_rtl8139};

void find_drivers(){
    for(int i = 0; i < pci_device_count; i++){
        for(int j = 0; j < sizeof(pci_drivers) / sizeof(pci_drivers[0]); j++){
            if(pci_drivers[j](&pci_devices[i])){
                outb(0x3F8, 'D');
                break;
            }
        }
    }
}

void pci_init()
{
    pci_scan_bus();
    find_drivers();
}