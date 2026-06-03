
OBJS += \
    $(BUILD)/kernel/syscall/syscall.o

$(BUILD)/kernel/syscall/%.o: kernel/syscall/%.c | $(BUILD)/kernel/syscall
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/syscall:
	mkdir -p $@
