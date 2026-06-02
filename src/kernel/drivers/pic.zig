const std = @import("std");

/// PIC (Programmable Interrupt Controller) driver
pub const PICDriver = struct {
    pub fn init() !void {
        // ICW1 - Initialize PIC1
        outb(0x20, 0x11);
        // ICW2 - Set offset to 32
        outb(0x21, 0x20);
        // ICW3 - Configure master/slave
        outb(0x21, 0x04);
        // ICW4 - 8086 mode
        outb(0x21, 0x01);
        // OCW1 - Enable all interrupts
        outb(0x21, 0x00);

        // ICW1 - Initialize PIC2
        outb(0xA0, 0x11);
        // ICW2 - Set offset to 40
        outb(0xA1, 0x28);
        // ICW3 - Configure slave
        outb(0xA1, 0x02);
        // ICW4 - 8086 mode
        outb(0xA1, 0x01);
        // OCW1 - Enable all interrupts
        outb(0xA1, 0x00);
    }

    pub fn enableIRQ(irq: u8) void {
        if (irq < 8) {
            outb(0x21, inb(0x21) & ~@as(u8, 1 << irq));
        } else {
            outb(0xA1, inb(0xA1) & ~@as(u8, 1 << (irq - 8)));
        }
    }

    pub fn disableIRQ(irq: u8) void {
        if (irq < 8) {
            outb(0x21, inb(0x21) | (@as(u8, 1) << irq));
        } else {
            outb(0xA1, inb(0xA1) | (@as(u8, 1) << (irq - 8)));
        }
    }
};

pub fn init() !void {
    try PICDriver.init();
}

fn outb(port: u16, value: u8) void {
    asm volatile (
        \\  outb %[val], %[port]
        : [port] "N{}" (port),
          [val] "{al}" (value)
    );
}

fn inb(port: u16) u8 {
    var value: u8 = 0;
    asm volatile (
        \\  inb %[port], %[val]
        : [val] "={al}" (value)
        : [port] "N{}" (port)
    );
    return value;
}
