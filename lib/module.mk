
OBJS += \
    $(BUILD)/lib/string.o \
    $(BUILD)/lib/printk.o \
    $(BUILD)/lib/sha256.o \
    $(BUILD)/lib/errno.o

$(BUILD)/lib/%.o: lib/%.c | $(BUILD)/lib
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/lib:
	mkdir -p $@
