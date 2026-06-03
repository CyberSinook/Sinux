OBJS += \
    $(BUILD)/drivers/vga.o      \
    $(BUILD)/drivers/serial.o   \
    $(BUILD)/drivers/tty.o      \
    $(BUILD)/drivers/keyboard.o \
    $(BUILD)/drivers/ata.o

$(BUILD)/drivers/%.o: drivers/%.c | $(BUILD)/drivers
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/drivers:
	mkdir -p $@
