const std = @import("std");

pub const InterruptHandler = struct {
    handlers: std.ArrayList(*const fn () void),
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator) !InterruptHandler {
        return InterruptHandler{
            .handlers = std.ArrayList(*const fn () void).init(allocator),
            .allocator = allocator,
        };
    }

    pub fn deinit(self: *InterruptHandler) void {
        self.handlers.deinit();
    }

    pub fn registerHandler(self: *InterruptHandler, handler: *const fn () void) !void {
        try self.handlers.append(handler);
    }
};

pub const Interrupts = struct {
    pub fn init() !void {
        // Enable interrupts
        asm volatile ("sti");
    }

    pub fn disable() void {
        asm volatile ("cli");
    }
};
