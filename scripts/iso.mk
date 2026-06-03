ISO_STAGE := $(BUILD)/iso/stage
ISO        := $(BUILD)/iso/sinux.iso
DISK       := sinux.img

iso: $(TARGET)
	mkdir -p $(ISO_STAGE)/boot/grub
	cp $(TARGET)     $(ISO_STAGE)/boot/kernel.elf
	cp boot/grub.cfg $(ISO_STAGE)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_STAGE)
	@echo "[OK] $(ISO)"
	@./scripts/mkdisk.sh
