const std = @import("std");
const drivers = @import("../drivers/mod.zig");

pub const Direction = enum {
    up,
    down,
    left,
    right,
    none,
};

pub const GhostState = enum {
    normal,
    frightened,
    eaten,
};

pub const Ghost = struct {
    x: i32,
    y: i32,
    direction: Direction,
    state: GhostState,
    color: u8,
};

pub const Player = struct {
    x: i32,
    y: i32,
    direction: Direction,
    next_direction: Direction,
    lives: i32,
    score: u32,
    pellets_eaten: u32,
    power_timer: u32,
};

pub const PacmanGame = struct {
    maze: [31][28]u8,
    player: Player,
    ghosts: [4]Ghost,
    state: u8,
    level: u8,
    total_pellets: u32,
    frame_count: u32,
    allocator: std.mem.Allocator,

    const GRID_WIDTH = 28;
    const GRID_HEIGHT = 31;
    const MAX_GHOSTS = 4;

    pub fn create(allocator: std.mem.Allocator) !PacmanGame {
        var game = PacmanGame{
            .maze = undefined,
            .player = Player{
                .x = 13,
                .y = 23,
                .direction = .none,
                .next_direction = .none,
                .lives = 3,
                .score = 0,
                .pellets_eaten = 0,
                .power_timer = 0,
            },
            .ghosts = undefined,
            .state = 1, // running
            .level = 1,
            .total_pellets = 0,
            .frame_count = 0,
            .allocator = allocator,
        };

        // Initialize ghosts
        game.ghosts[0] = Ghost{ .x = 13, .y = 11, .direction = .left, .state = .normal, .color = 1 };
        game.ghosts[1] = Ghost{ .x = 12, .y = 14, .direction = .up, .state = .normal, .color = 2 };
        game.ghosts[2] = Ghost{ .x = 14, .y = 14, .direction = .down, .state = .normal, .color = 3 };
        game.ghosts[3] = Ghost{ .x = 13, .y = 13, .direction = .right, .state = .normal, .color = 4 };

        // Initialize maze with pellets
        @memset(&game.maze, 2); // All pellets
        game.total_pellets = 240;

        return game;
    }

    pub fn update(self: *PacmanGame) void {
        self.frame_count += 1;

        if (self.frame_count % 2 == 0) {
            self.movePlayer();
        }

        if (self.frame_count % 3 == 0) {
            for (0..MAX_GHOSTS) |i| {
                self.moveGhost(i);
            }
        }
    }

    pub fn movePlayer(self: *PacmanGame) void {
        var new_x = self.player.x;
        var new_y = self.player.y;

        switch (self.player.direction) {
            .up => if (new_y > 0) new_y -= 1,
            .down => if (new_y < GRID_HEIGHT - 1) new_y += 1,
            .left => if (new_x > 0) new_x -= 1,
            .right => if (new_x < GRID_WIDTH - 1) new_x += 1,
            .none => {},
        }

        self.player.x = new_x;
        self.player.y = new_y;

        // Eat pellet
        if (self.maze[@intCast(self.player.y)][@intCast(self.player.x)] == 2) {
            self.maze[@intCast(self.player.y)][@intCast(self.player.x)] = 0;
            self.player.score += 10;
            self.player.pellets_eaten += 1;
        }
    }

    pub fn moveGhost(self: *PacmanGame, ghost_idx: usize) void {
        if (ghost_idx >= MAX_GHOSTS) return;
        var ghost = &self.ghosts[ghost_idx];

        // Simple AI: move towards player
        if (ghost.x < self.player.x and ghost.x < GRID_WIDTH - 1) {
            ghost.x += 1;
        } else if (ghost.x > self.player.x and ghost.x > 0) {
            ghost.x -= 1;
        } else if (ghost.y < self.player.y and ghost.y < GRID_HEIGHT - 1) {
            ghost.y += 1;
        } else if (ghost.y > self.player.y and ghost.y > 0) {
            ghost.y -= 1;
        }
    }

    pub fn render(self: *PacmanGame) !void {
        var buf: [256]u8 = undefined;
        const msg = try std.fmt.bufPrint(&buf, "[PACMAN] Score: {d} Lives: {d}\n", .{ self.player.score, self.player.lives });
        try drivers.serial.write(msg);
    }
};

var game: ?PacmanGame = null;

pub fn serviceStart() !void {
    try drivers.serial.print("[PACMAN] Starting Pacman Game Service...\n", .{});
    // Game would be initialized here
}

pub fn serviceStop() !void {
    try drivers.serial.print("[PACMAN] Stopping Pacman Game Service\n", .{});
}
