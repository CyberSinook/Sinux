const std = @import("std");

pub const cpu = @import("cpu/mod.zig");
pub const interrupts = @import("interrupts/mod.zig");

pub const X86Architecture = struct {
    cpu_instance: cpu.CPU,
    interrupt_handler: interrupts.InterruptHandler,

    pub fn init(allocator: std.mem.Allocator) !X86Architecture {
        return X86Architecture{
            .cpu_instance = try cpu.CPU.init(allocator),
            .interrupt_handler = try interrupts.InterruptHandler.init(allocator),
        };
    }
};
