# BSD Bootloader Documentation

## Overview

The Sinux OS now includes a BSD-compatible bootloader that:
- Boots x86 systems in real mode
- Enables A20 line for extended memory access
- Loads the Zig kernel from disk
- Prepares BSD boot arguments
- Enters protected mode and transfers control to kernel

## Features

### Real Mode Operations
- **Boot Drive Detection** - Automatically detects boot device
- **A20 Line Enable** - Supports both BIOS and keyboard controller methods
- **Disk I/O** - Reads kernel from disk using BIOS interrupts
- **Memory Detection** - Queries BIOS for memory information

### Protected Mode Transition
- **GDT Setup** - Global Descriptor Table with code/data segments
- **CR0 Configuration** - Enables protected mode via CR0.PE bit
- **Segment Initialization** - Sets up 32-bit segment registers
- **Stack Setup** - Initializes 32-bit stack at 0x200000

### BSD Boot Arguments
The bootloader prepares a boot arguments structure at 0x7000:
- Magic number (0x57494E42 - 'WINB')
- Boot device information
- Boot flags
- Kernel entry point
- Lower memory size
- Upper memory size
- Extended memory size

## Memory Layout

```
0x00000000 - 0x000FFFFF    Real mode / Low memory (1MB)
0x00007C00 - 0x000007FF    Bootloader (512 bytes)
0x00007000 - 0x00007FFF    BSD boot arguments
0x00001000 - 0x0000EFFF    Kernel initial load
0x00100000+                 High memory / Kernel execution
```

## Building

```bash
# Build bootloader
./build_tools/build_bootloader.sh

# Output: bsd_bootloader.bin (512 bytes max)
```

## Creating Bootable Image

```bash
# Create blank floppy image
dd if=/dev/zero of=sinux.img bs=512 count=2880

# Write bootloader
dd if=bsd_bootloader.bin of=sinux.img bs=512 count=1 conv=notrunc

# Write kernel (starting at sector 2)
dd if=kernel.bin of=sinux.img bs=512 seek=1 conv=notrunc

# Boot with QEMU
qemu-system-i386 -fda sinux.img
```

## Boot Sequence

1. BIOS loads bootloader from sector 0
2. Bootloader initialization (16-bit real mode)
3. Enable A20 line for >1MB memory access
4. Load kernel from disk (sectors 2+)
5. Prepare BSD boot arguments structure
6. Load GDT with code/data descriptors
7. Enter protected mode (32-bit)
8. Jump to kernel at 0x100000

## Error Handling

- **A20 Error**: Failed to enable extended memory line
  - Fallback methods attempted
  - System halts if unsuccessful

- **Load Error**: Kernel loading failed
  - Disk read error
  - Invalid sector number

## Compatibility

- **BIOS Systems**: Full support
- **UEFI Systems**: Not supported (use EFI bootloader)
- **Boot Devices**: Floppy disk, hard drive, USB (via BIOS)
- **Architecture**: x86 (32-bit)

## Future Enhancements

- UEFI support
- Network boot (PXE)
- Multi-boot specification compliance
- Kernel decompression
- Boot menu support
