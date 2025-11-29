# کامپایلر و ابزارها
CC = gcc
LD = ld
NASM = nasm
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -O0 -fno-builtin -fno-stack-protector
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib
ISO_DIR = isodir
KERNEL = kernel.bin
ISO = my_kernel.iso
QEMU = qemu-system-i386

# فایل‌های ماژول‌ها
MEMORY_MANAGER_C = kernel/memory_manager/memory_manager.c
MEMORY_MANAGER = kernel/memory_manager/memory_manager.o

IDT_C = kernel/IDT/IDT.c
IDT_ASM = kernel/IDT/IDT_asm.asm
IDT = kernel/IDT/IDT.o kernel/IDT/IDT_asm.o

# کرنل اصلی
KERNEL_OBJ = kernel.o

# همه ماژول‌ها
MODULES = $(MEMORY_MANAGER) $(IDT)

# --------------------------
# کامپایل فایل‌های C
# --------------------------
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# کامپایل فایل‌های ASM با NASM
%.o: %.asm
	$(NASM) -f elf32 $< -o $@

# --------------------------
# کامپایل کرنل اصلی
# --------------------------
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@
	g++ ramfs.cpp -o ramfs_creator

# --------------------------
# لینک کردن کرنل و ماژول‌ها
# --------------------------
$(KERNEL): $(KERNEL_OBJ) $(MODULES)
	$(LD) $(LDFLAGS) $^ -o $@

# --------------------------
# ساخت ISO
# --------------------------
setup_iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/
	./ramfs_creator
	cp $(MEMORY_MANAGER) $(ISO_DIR)/boot/
	cp $(IDT) $(ISO_DIR)/boot/
	echo 'menuentry "SINUX Project" { multiboot /boot/kernel.bin; module /boot/ramfs; module /boot/memory_manager.o; module /boot/IDT.o; boot; }' > $(ISO_DIR)/boot/grub/grub.cfg

$(ISO): setup_iso
	grub-mkrescue -o $@ $(ISO_DIR)

# --------------------------
# اجرای QEMU
# --------------------------
run: $(ISO)
	$(QEMU) -cdrom $(ISO)
# 	$(QEMU) -cdrom $(ISO) -full-screen

# --------------------------
# پاکسازی
# --------------------------
clean:
	rm -rf *.o $(KERNEL) $(ISO) $(ISO_DIR) kernel/memory_manager/memory_manager.o kernel/IDT/IDT_asm.o kernel/IDT/IDT.o ramfs_creator

.PHONY: all run clean setup_iso