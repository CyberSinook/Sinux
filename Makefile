CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -O0 -fno-builtin -fno-stack-protector
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib
ISO_DIR = isodir
KERNEL = sinux-kernel
ISO = sinux.iso
QEMU = qemu-system-x86_64

all: $(ISO)

kernel.o: kernel.c
	nasm -f elf multiboot.asm -o multiboot.o
	$(CC) $(CFLAGS) -c $< -o $@

#link
$(KERNEL): kernel.o
	$(LD) $(LDFLAGS) $^ multiboot.o -o $@

# iso ready
setup_iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg

# build iso
$(ISO): setup_iso
	grub-mkrescue -o $@ $(ISO_DIR)

# run
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)


clean:
	rm -rf *.o $(KERNEL) $(ISO) $(ISO_DIR)

.PHONY: all run clean
