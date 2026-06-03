#!/bin/bash
DISK="sinux.img"
SIZE_MB=128

if [ -f "$DISK" ]; then
    echo "[OK] $DISK already exists, skipping"
    exit 0
fi

echo "[..] Creating $DISK ($SIZE_MB MiB)..."
dd if=/dev/zero of=$DISK bs=1M count=$SIZE_MB status=none
mkfs.ext2 -L "sinux" -b 1024 $DISK
echo "[OK] $DISK created"
