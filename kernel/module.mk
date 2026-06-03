
OBJS += \
    $(BUILD)/kernel/core/main.o       \
    $(BUILD)/kernel/core/panic.o      \
    $(BUILD)/kernel/core/multiboot.o  \
    $(BUILD)/kernel/proc/process.o    \
    $(BUILD)/kernel/proc/scheduler.o  \
    $(BUILD)/kernel/fs/vfs.o          \
    $(BUILD)/kernel/fs/ramfs.o        \
    $(BUILD)/kernel/syscall/syscall.o

$(BUILD)/kernel/core/%.o: kernel/core/%.c | $(BUILD)/kernel/core
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/proc/%.o: kernel/proc/%.c | $(BUILD)/kernel/proc
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/fs/%.o: kernel/fs/%.c | $(BUILD)/kernel/fs
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/syscall/%.o: kernel/syscall/%.c | $(BUILD)/kernel/syscall
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/core:    ; mkdir -p $@
$(BUILD)/kernel/proc:    ; mkdir -p $@
$(BUILD)/kernel/fs:      ; mkdir -p $@
$(BUILD)/kernel/syscall: ; mkdir -p $@
