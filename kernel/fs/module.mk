OBJS += \
    $(BUILD)/kernel/fs/vfs.o     \
    $(BUILD)/kernel/fs/ramfs.o   \
    $(BUILD)/kernel/fs/ext2.o    \
    $(BUILD)/kernel/fs/procfs.o

$(BUILD)/kernel/fs/%.o: kernel/fs/%.c | $(BUILD)/kernel/fs
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/fs:
	mkdir -p $@
