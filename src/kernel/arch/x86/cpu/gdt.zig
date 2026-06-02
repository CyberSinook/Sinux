const std = @import("std");

/// Global Descriptor Table
pub const GDT = struct {
    entries: [6]GDTEntry,
    limit: u16,
    allocator: std.mem.Allocator,

    pub const GDTEntry = extern struct {
        limit_low: u16,
        base_low: u16,
        base_mid: u8,
        access: u8,
        granularity: u8,
        base_high: u8,
    };

    pub fn init(allocator: std.mem.Allocator) !GDT {
        var gdt = GDT{
            .entries = undefined,
            .limit = @sizeOf([6]GDTEntry) - 1,
            .allocator = allocator,
        };

        // Null descriptor
        gdt.entries[0] = GDTEntry{
            .limit_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .access = 0,
            .granularity = 0,
            .base_high = 0,
        };

        // Kernel code segment
        gdt.entries[1] = GDTEntry{
            .limit_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .access = 0x9A,
            .granularity = 0xCF,
            .base_high = 0,
        };

        // Kernel data segment
        gdt.entries[2] = GDTEntry{
            .limit_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .access = 0x92,
            .granularity = 0xCF,
            .base_high = 0,
        };

        // User code segment
        gdt.entries[3] = GDTEntry{
            .limit_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .access = 0xFA,
            .granularity = 0xCF,
            .base_high = 0,
        };

        // User data segment
        gdt.entries[4] = GDTEntry{
            .limit_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .access = 0xF2,
            .granularity = 0xCF,
            .base_high = 0,
        };

        // TSS descriptor (for task switching)
        gdt.entries[5] = GDTEntry{
            .limit_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .access = 0x89,
            .granularity = 0x40,
            .base_high = 0,
        };

        return gdt;
    }

    pub fn deinit(self: *GDT) void {
        _ = self;
    }

    pub fn load(self: *const GDT) void {
        const gdtr = GDTRegister{
            .limit = self.limit,
            .base = @intFromPtr(&self.entries[0]),
        };

        asm volatile (
            \\  lgdt (%[gdtr])
            : [gdtr] "r" (&gdtr)
        );
    }
};

pub const GDTRegister = extern struct {
    limit: u16,
    base: u32,
};
