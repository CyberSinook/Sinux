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

    /// Print the rainbow banner with LGBT colors
    pub fn printRainbowBanner(self: *Shell) !void {
        // ANSI color codes for LGBT rainbow
        const color_red = "\x1b[38;5;196m";      // Red
        const color_orange = "\x1b[38;5;208m";   // Orange
        const color_yellow = "\x1b[38;5;226m";   // Yellow
        const color_green = "\x1b[38;5;46m";     // Green
        const color_blue = "\x1b[38;5;33m";      // Blue
        const color_purple = "\x1b[38;5;129m";   // Purple
        const color_reset = "\x1b[0m";           // Reset color

        // Clear screen
        try drivers.serial.print("\x1b[2J\x1b[H", .{});

        // Rainbow top border
        try drivers.serial.write(color_red);
        try drivers.serial.write("╔════════════════════════════════════════════╗\n");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("║                                            ║\n");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write("║                                            ║\n");
        try drivers.serial.write(color_green);
        try drivers.serial.write("║                                            ║\n");
        try drivers.serial.write(color_blue);
        try drivers.serial.write("║                                            ║\n");
        try drivers.serial.write(color_purple);
        try drivers.serial.write("║                                            ║\n");

        // Rainbow SINUX text with each letter in different color
        try drivers.serial.write(color_red);
        try drivers.serial.write("║  ");
        try drivers.serial.write(color_red);
        try drivers.serial.write("███████╗");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("██╗");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write("    ██╗");
        try drivers.serial.write(color_green);
        try drivers.serial.write("██╗   ██╗██╗   ██╗██╗  ██╗");
        try drivers.serial.write(color_blue);
        try drivers.serial.write("  ║\n");

        try drivers.serial.write(color_red);
        try drivers.serial.write("║  ");
        try drivers.serial.write(color_red);
        try drivers.serial.write("██╔════╝");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("██║");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write("    ██║");
        try drivers.serial.write(color_green);
        try drivers.serial.write("██║   ██║██║   ██║╚██╗██╔╝");
        try drivers.serial.write(color_blue);
        try drivers.serial.write("  ║\n");

        try drivers.serial.write(color_red);
        try drivers.serial.write("║  ");
        try drivers.serial.write(color_red);
        try drivers.serial.write("███████╗");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("██║");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write("    ██║");
        try drivers.serial.write(color_green);
        try drivers.serial.write("██║   ██║██║   ██║ ╚███╔╝");
        try drivers.serial.write(color_blue);
        try drivers.serial.write("   ║\n");

        try drivers.serial.write(color_red);
        try drivers.serial.write("║  ");
        try drivers.serial.write(color_red);
        try drivers.serial.write("╚════██║");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("██║");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write("    ██║");
        try drivers.serial.write(color_green);
        try drivers.serial.write("██║   ██║██║   ██║ ██╔██╗");
        try drivers.serial.write(color_blue);
        try drivers.serial.write("   ║\n");

        try drivers.serial.write(color_red);
        try drivers.serial.write("║  ");
        try drivers.serial.write(color_red);
        try drivers.serial.write("███████║");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("███████╗");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write("███████╗");
        try drivers.serial.write(color_green);
        try drivers.serial.write("╚██████╔╝╚██████╔╝██╔╝ ██╗");
        try drivers.serial.write(color_blue);
        try drivers.serial.write("  ║\n");

        try drivers.serial.write(color_red);
        try drivers.serial.write("║  ");
        try drivers.serial.write(color_red);
        try drivers.serial.write("╚══════╝");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("╚══════╝");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write("╚══════╝");
        try drivers.serial.write(color_green);
        try drivers.serial.write(" ╚═════╝  ╚═════╝ ╚═╝  ╚═╝");
        try drivers.serial.write(color_blue);
        try drivers.serial.write("  ║\n");

        // Rainbow separator
        try drivers.serial.write(color_purple);
        try drivers.serial.write("║                                            ║\n");
        try drivers.serial.write(color_red);
        try drivers.serial.write("║    ");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("🏳️‍🌈 Zig Edition - Pride in Code 🏳️‍🌈");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write("     ║\n");
        try drivers.serial.write(color_green);
        try drivers.serial.write("║                                            ║\n");

        // Rainbow bottom border
        try drivers.serial.write(color_blue);
        try drivers.serial.write("║                                            ║\n");
        try drivers.serial.write(color_purple);
        try drivers.serial.write("║                                            ║\n");
        try drivers.serial.write(color_red);
        try drivers.serial.write("╚════════════════════════════════════════════╝\n");

        // Welcome messages in rainbow
        try drivers.serial.write(color_red);
        try drivers.serial.write("\n✨ ");
        try drivers.serial.write(color_orange);
        try drivers.serial.write("Welcome to Sinux Shell");
        try drivers.serial.write(color_yellow);
        try drivers.serial.write(" (Zig Edition)");
        try drivers.serial.write(color_green);
        try drivers.serial.write(" ✨\n");

        try drivers.serial.write(color_blue);
        try drivers.serial.write("Type 'help' for available commands\n");
        try drivers.serial.write(color_purple);
        try drivers.serial.write("Love is Love - Code with Pride!\n");
        try drivers.serial.write(color_reset);
        try drivers.serial.write("\n");
    }

    pub fn run(self: *Shell) !void {
        self.running = true;
        
        // Print the rainbow banner
        try self.printRainbowBanner();

        // Shell loop would go here
        // For now, just demonstrate it works
    }
};
