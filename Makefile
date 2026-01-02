ARCH     = x86
CC       = gcc
AS       = nasm
LD       = ld

CFLAGS   = -m32 -ffreestanding -O2 -Wall -Wextra -Iinclude -fno-stack-protector
ASFLAGS  = -felf32
LDFLAGS  = -m elf_i386 -T linker.ld

BUILD    = build
OBJDIR   = $(BUILD)/obj
ISODIR   = $(BUILD)/iso

SRC_C    := $(shell find . -type f -name "*.c")
SRC_ASM  := $(shell find . -type f -name "*.asm")

OBJ_C    := $(patsubst ./%, $(OBJDIR)/%, $(SRC_C:.c=.o))
OBJ_ASM  := $(patsubst ./%, $(OBJDIR)/%, $(SRC_ASM:.asm=.o))

OBJ      := $(OBJ_C) $(OBJ_ASM)

all: $(BUILD)/Sinux-0.03.iso

$(BUILD)/Sinux-0.03.elf: $(OBJ)
	@mkdir -p $(BUILD)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/Sinux-0.03.iso: $(BUILD)/Sinux-0.03.elf
	@mkdir -p $(ISODIR)/boot/grub
	cp $< $(ISODIR)/boot/Sinux-0.03.elf
	cp boot/grub/grub.cfg $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISODIR) >/dev/null 2>&1

clean:
	rm -rf $(BUILD)

run: all
	qemu-system-i386 -cdrom $(BUILD)/Sinux-0.03.iso -m 128M -netdev user,id=net0 -device rtl8139,netdev=net0
