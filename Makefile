ARCH     = x86
CC       = gcc
AS       = nasm
LD       = ld
CFLAGS   = -m32 -ffreestanding -O2 -Wall -Wextra -Iinclude
ASFLAGS  = -felf32
LDFLAGS  = -m elf_i386 -T linker.ld

SRC_C    = $(shell find . -name "*.c")
SRC_ASM  = $(shell find . -name "*.asm")

OBJ_C    = $(SRC_C:.c=.o)
OBJ_ASM  = $(SRC_ASM:.asm=.o)

OBJ      = $(OBJ_ASM) $(OBJ_C)

all: kernel.iso

kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

kernel.iso: kernel.elf
	mkdir -p build/iso/boot/grub
	cp kernel.elf build/iso/boot/kernel.elf
	cp boot/grub/grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o kernel.iso build/iso

clean:
	rm -f $(OBJ) kernel.elf kernel.iso
	rm -rf build/iso

run: clean all
	qemu-system-i386 -cdrom kernel.iso -m 128M
