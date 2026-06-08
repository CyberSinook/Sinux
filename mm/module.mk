
OBJS += \
    $(BUILD)/mm/pmm.o \
    $(BUILD)/mm/vmm.o \
    $(BUILD)/mm/slab.o

$(BUILD)/mm/%.o: mm/%.c | $(BUILD)/mm
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/mm:
	mkdir -p $@
