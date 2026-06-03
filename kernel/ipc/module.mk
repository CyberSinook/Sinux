OBJS += \
    $(BUILD)/kernel/ipc/pipe.o   \
    $(BUILD)/kernel/ipc/signal.o

$(BUILD)/kernel/ipc/%.o: kernel/ipc/%.c | $(BUILD)/kernel/ipc
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/ipc:
	mkdir -p $@
