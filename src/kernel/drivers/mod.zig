const std = @import("std");

pub const serial = @import("serial.zig");
pub const pic = @import("pic.zig");

pub const Drivers = struct {
    pub fn init(allocator: std.mem.Allocator) !void {
        try serial.init();
        try pic.init();
    }
};
