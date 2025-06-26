CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -O0 -fno-builtin -fno-stack-protector
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib
ISO_DIR = isodir
KERNEL = kernel.bin
ISO = my_kernel.iso
QEMU = qemu-system-x86_64

all: $(ISO)

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

#link
$(KERNEL): kernel.o
	$(LD) $(LDFLAGS) $^ -o $@

# iso ready
setup_iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/
	echo 'menuentry "SINUX Project" { multiboot /boot/kernel.bin; boot; }' > $(ISO_DIR)/boot/grub/grub.cfg

# build iso
$(ISO): setup_iso
	grub-mkrescue -o $@ $(ISO_DIR)

# run
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)


clean:
	rm -rf *.o $(KERNEL) $(ISO) $(ISO_DIR)

.PHONY: all run clean
