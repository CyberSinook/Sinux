const std = @import("std");

pub const gdt = @import("gdt.zig");
pub const idt = @import("idt.zig");
pub const interrupts = @import("interrupts.zig");

pub const CPU = struct {
    gdt_instance: gdt.GDT,
    idt_instance: idt.IDT,
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator) !CPU {
        return CPU{
            .gdt_instance = try gdt.GDT.init(allocator),
            .idt_instance = try idt.IDT.init(allocator),
            .allocator = allocator,
        };
    }

    pub fn deinit(self: *CPU) void {
        self.gdt_instance.deinit();
        self.idt_instance.deinit();
    }
};
