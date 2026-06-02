const std = @import("std");

pub const init_mod = @import("init.zig");
pub const shell = @import("shell.zig");

pub const System = struct {
    init_system: init_mod.InitSystem,

    pub fn create(allocator: std.mem.Allocator) !System {
        return System{
            .init_system = try init_mod.InitSystem.create(allocator),
        };
    }
};
