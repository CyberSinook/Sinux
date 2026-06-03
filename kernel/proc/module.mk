OBJS += \
    $(BUILD)/kernel/proc/process.o   \
    $(BUILD)/kernel/proc/scheduler.o \
    $(BUILD)/kernel/proc/elf.o       \
    $(BUILD)/kernel/proc/usermode.o

$(BUILD)/kernel/proc/%.o: kernel/proc/%.c | $(BUILD)/kernel/proc
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/proc:
	mkdir -p $@
