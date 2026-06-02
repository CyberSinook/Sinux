#!/bin/bash
# BSD Bootloader Build Script

echo "Building BSD Bootloader for Sinux OS..."

# Assemble bootloader
echo "Assembling bootloader..."
as -o bsd_bootloader.o src/boot/bsd_bootloader.s

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to assemble bootloader"
    exit 1
fi

# Link bootloader
echo "Linking bootloader..."
ld -Ttext 0x7C00 -o bsd_bootloader.elf bsd_bootloader.o

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to link bootloader"
    exit 1
fi

# Extract binary
echo "Extracting bootloader binary..."
objcopy -O binary bsd_bootloader.elf bsd_bootloader.bin

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to extract bootloader binary"
    exit 1
fi

# Verify bootloader size
BOOTLOADER_SIZE=$(stat -f%z bsd_bootloader.bin 2>/dev/null || stat -c%s bsd_bootloader.bin)
echo "Bootloader size: $BOOTLOADER_SIZE bytes"

if [ $BOOTLOADER_SIZE -gt 512 ]; then
    echo "ERROR: Bootloader exceeds 512 bytes"
    exit 1
fi

echo "BSD Bootloader built successfully!"
echo "Output: bsd_bootloader.bin"
