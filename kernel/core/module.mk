OBJS += \
    $(BUILD)/kernel/core/main.o      \
    $(BUILD)/kernel/core/panic.o     \
    $(BUILD)/kernel/core/multiboot.o

$(BUILD)/kernel/core/%.o: kernel/core/%.c | $(BUILD)/kernel/core
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/core:
	mkdir -p $@
