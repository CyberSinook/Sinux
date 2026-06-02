const std = @import("std");

/// Interrupt Descriptor Table
pub const IDT = struct {
    entries: [256]IDTEntry,
    limit: u16,
    allocator: std.mem.Allocator,

    pub const IDTEntry = extern struct {
        offset_low: u16,
        selector: u16,
        zero: u8,
        type_attr: u8,
        offset_high: u16,
    };

    pub fn init(allocator: std.mem.Allocator) !IDT {
        var idt = IDT{
            .entries = undefined,
            .limit = @sizeOf([256]IDTEntry) - 1,
            .allocator = allocator,
        };

        @memset(&idt.entries, @as(IDTEntry, @bitCast(@as(u64, 0))));

        return idt;
    }

    pub fn deinit(self: *IDT) void {
        _ = self;
    }

    pub fn setGate(self: *IDT, index: u8, handler: u32, selector: u16, type_attr: u8) void {
        self.entries[index] = IDTEntry{
            .offset_low = @as(u16, @truncate(handler)),
            .selector = selector,
            .zero = 0,
            .type_attr = type_attr,
            .offset_high = @as(u16, @truncate(handler >> 16)),
        };
    }

    pub fn load(self: *const IDT) void {
        const idtr = IDTRegister{
            .limit = self.limit,
            .base = @intFromPtr(&self.entries[0]),
        };

        asm volatile (
            \\  lidt (%[idtr])
            : [idtr] "r" (&idtr)
        );
    }
};

pub const IDTRegister = extern struct {
    limit: u16,
    base: u32,
};
