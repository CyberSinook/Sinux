const std = @import("std");

/// Serial driver for console output
pub const SerialDriver = struct {
    port: u16 = 0x3F8,

    pub fn init() !void {
        // Initialize serial port
        // Disable interrupts
        outb(0x3F8 + 1, 0x00);
        // Enable DLAB
        outb(0x3F8 + 3, 0x80);
        // Set divisor to 1 (115200 baud)
        outb(0x3F8 + 0, 0x01);
        outb(0x3F8 + 1, 0x00);
        // Disable DLAB, set 8 bits
        outb(0x3F8 + 3, 0x03);
        // Enable FIFO
        outb(0x3F8 + 2, 0xC7);
        // Set RTS/DTR
        outb(0x3F8 + 4, 0x0B);
    }

    pub fn putChar(char: u8) void {
        // Wait for transmit buffer to be empty
        while ((inb(0x3F8 + 5) & 0x20) == 0) {}
        outb(0x3F8, char);
    }

    pub fn write(data: []const u8) void {
        for (data) |byte| {
            putChar(byte);
        }
    }
};

var driver = SerialDriver{};

pub fn init() !void {
    try driver.init();
}

pub fn putChar(char: u8) void {
    driver.putChar(char);
}

pub fn write(data: []const u8) !void {
    driver.write(data);
}

pub fn print(comptime format: []const u8, args: anytype) !void {
    var buf: [256]u8 = undefined;
    const formatted = try std.fmt.bufPrint(&buf, format, args);
    try write(formatted);
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
