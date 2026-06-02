const std = @import("std");
const drivers = @import("../drivers/mod.zig");

pub const ShellCommand = struct {
    name: []const u8,
    description: []const u8,
    handler: *const fn ([][]const u8) anyerror!void,
};

pub const Shell = struct {
    commands: std.ArrayList(ShellCommand),
    history: std.ArrayList([]const u8),
    allocator: std.mem.Allocator,
    running: bool = false,

    pub fn create(allocator: std.mem.Allocator) !Shell {
        return Shell{
            .commands = std.ArrayList(ShellCommand).init(allocator),
            .history = std.ArrayList([]const u8).init(allocator),
            .allocator = allocator,
        };
    }

    pub fn destroy(self: *Shell) void {
        self.commands.deinit();
        self.history.deinit();
    }

    pub fn registerCommand(self: *Shell, cmd: ShellCommand) !void {
        try self.commands.append(cmd);
    }

    pub fn execute(self: *Shell, input: []const u8) !void {
        var it = std.mem.splitSequence(u8, input, " ");
        const cmd_name = it.next() orelse return;

        for (self.commands.items) |cmd| {
            if (std.mem.eql(u8, cmd.name, cmd_name)) {
                var args = std.ArrayList([]const u8).init(self.allocator);
                defer args.deinit();
                while (it.next()) |arg| {
                    try args.append(arg);
                }
                try cmd.handler(args.items);
                return;
            }
        }
        try drivers.serial.print("Command not found: {s}\n", .{cmd_name});
    }

    pub fn run(self: *Shell) !void {
        self.running = true;
        try drivers.serial.print("\n=== SINUX SHELL (Zig Edition) ===\n", .{});
        try drivers.serial.print("Type 'help' for commands\n\n", .{});

        // Shell loop would go here
        // For now, just demonstrate it works
    }
};
