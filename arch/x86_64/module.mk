
ARCH_DIR := arch/x86_64

OBJS += \
    $(BUILD)/arch/x86_64/boot.o        \
    $(BUILD)/arch/x86_64/gdt.o         \
    $(BUILD)/arch/x86_64/idt.o         \
    $(BUILD)/arch/x86_64/pic.o         \
    $(BUILD)/arch/x86_64/pit.o         \
    $(BUILD)/arch/x86_64/syscall_init.o

$(BUILD)/arch/x86_64/boot.o: arch/x86_64/boot.asm | $(BUILD)/arch/x86_64
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD)/arch/x86_64/%.o: arch/x86_64/%.c | $(BUILD)/arch/x86_64
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/arch/x86_64:
	mkdir -p $@
