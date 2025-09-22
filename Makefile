CC = x86_64-elf-gcc
LD = x86_64-elf-ld
CFLAGS = -ffreestanding -m64 -Wall -Wextra -Ikernel/include
LDFLAGS = -T linker.ld

BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso
EFI_DIR = $(ISO_DIR)/boot/grub
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
ISO_FILE = $(BUILD_DIR)/kernel.iso

KERNEL_C_SRC = $(wildcard kernel/**/*.c)
KERNEL_OBJ = $(patsubst kernel/%.c,$(BUILD_DIR)/%.o,$(KERNEL_C_SRC))

$(BUILD_DIR)/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(KERNEL_ELF) iso

$(KERNEL_ELF): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

iso: $(KERNEL_ELF)
	@mkdir -p $(EFI_DIR)
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	cp boot/grub/grub.cfg $(EFI_DIR)/
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)

run:
	qemu-system-x86_64 -cdrom build/kernel.iso

clean:
	find $(BUILD_DIR) -type f -delete

.PHONY: all clean iso
