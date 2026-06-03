OVMF      := edk2/OVMF.4m.fd
ISO       := $(BUILD)/iso/sinux.iso
DISK      := sinux.img
QEMU      := qemu-system-x86_64
QEMU_BASE := -cdrom $(ISO) -m 256M
QEMU_DISK := -drive file=$(DISK),format=raw,if=ide,index=0,media=disk
QEMU_DBG  := -d int,cpu_reset -D /tmp/sinux_qemu.log

run-uefi: iso
	@[ -f $(OVMF) ] || { echo "OVMF not found"; exit 1; }
	@[ -f $(DISK) ] || { echo "[ERR] $(DISK) not found."; exit 1; }
	@echo "[UEFI mode]"
	$(QEMU) $(QEMU_BASE) $(QEMU_DISK) -bios $(OVMF) -serial stdio -display sdl

run-bios: iso
	@[ -f $(DISK) ] || { echo "[ERR] $(DISK) not found."; exit 1; }
	@echo "[BIOS mode]"
	$(QEMU) $(QEMU_BASE) $(QEMU_DISK) -serial stdio -display sdl

run-debug: iso
	@[ -f $(DISK) ] || { echo "[ERR] $(DISK) not found."; exit 1; }
	@echo "[DEBUG mode] log -> /tmp/sinux_qemu.log"
	$(QEMU) $(QEMU_BASE) $(QEMU_DISK) $(QEMU_DBG) -serial stdio -display sdl
	@echo "--- last 30 lines of log ---"
	@tail -30 /tmp/sinux_qemu.log 2>/dev/null || true

run-serial: iso
	@[ -f $(DISK) ] || { echo "[ERR] $(DISK) not found."; exit 1; }
	$(QEMU) $(QEMU_BASE) $(QEMU_DISK) -nographic -serial mon:stdio

disk:
	@./scripts/mkdisk.sh
