const std = @import("std");
const drivers = @import("../drivers/mod.zig");

pub const PacmanConsole = struct {
    pub fn render(game: anytype) !void {
        try drivers.serial.print("\033[2J\033[H", .{});
        try drivers.serial.print("=== SINUX PACMAN ===\n", .{});
    }

    pub fn clear() !void {
        try drivers.serial.print("\033[2J\033[H", .{});
    }
};
