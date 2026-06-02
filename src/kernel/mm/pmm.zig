const std = @import("std");

/// Physical Memory Manager for Sinux
pub const PhysicalMemoryManager = struct {
    bitmap: std.ArrayList(u8),
    allocator: std.mem.Allocator,
    total_frames: u32,
    free_frames: u32,

    const FRAME_SIZE = 4096; // 4KB
    const BITMAP_SIZE = 32768; // 128MB addressable

    pub fn init(allocator: std.mem.Allocator) !PhysicalMemoryManager {
        var bitmap = std.ArrayList(u8).init(allocator);
        try bitmap.resize(BITMAP_SIZE);
        @memset(bitmap.items, 0);

        return PhysicalMemoryManager{
            .bitmap = bitmap,
            .allocator = allocator,
            .total_frames = BITMAP_SIZE * 8,
            .free_frames = BITMAP_SIZE * 8,
        };
    }

    pub fn deinit(self: *PhysicalMemoryManager) void {
        self.bitmap.deinit();
    }

    /// Allocate a physical frame
    pub fn allocateFrame(self: *PhysicalMemoryManager) !u32 {
        for (self.bitmap.items, 0..) |byte, i| {
            if (byte != 0xFF) {
                for (0..8) |bit| {
                    if ((byte & (@as(u8, 1) << @intCast(bit))) == 0) {
                        self.bitmap.items[i] |= @as(u8, 1) << @intCast(bit);
                        self.free_frames -= 1;
                        return (@as(u32, @intCast(i)) * 8 + @as(u32, @intCast(bit))) * FRAME_SIZE;
                    }
                }
            }
        }
        return error.OutOfMemory;
    }

    /// Free a physical frame
    pub fn freeFrame(self: *PhysicalMemoryManager, addr: u32) void {
        const frame = addr / FRAME_SIZE;
        const byte_idx = frame / 8;
        const bit_idx = frame % 8;

        if (byte_idx < self.bitmap.items.len) {
            self.bitmap.items[byte_idx] &= ~(@as(u8, 1) << @intCast(bit_idx));
            self.free_frames += 1;
        }
    }

    pub fn getFreeFrames(self: *const PhysicalMemoryManager) u32 {
        return self.free_frames;
    }
};
