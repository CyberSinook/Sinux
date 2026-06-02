const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var stdout = std.io.getStdOut().writer();
    try stdout.print("Sinux OS - Zig Edition Build System\n", .{});
    try stdout.print("Building kernel...\n", .{});
}
