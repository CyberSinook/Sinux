const std = @import("std");

/// Heap allocator for Sinux
pub const Heap = struct {
    allocator: std.mem.Allocator,
    base_addr: u32,
    size: u32,
    used: u32,

    const HEAP_SIZE = 0x100000; // 1MB

    pub fn init(allocator: std.mem.Allocator) !Heap {
        return Heap{
            .allocator = allocator,
            .base_addr = 0x200000, // 2MB
            .size = HEAP_SIZE,
            .used = 0,
        };
    }

    pub fn deinit(self: *Heap) void {
        // Cleanup if needed
        _ = self;
    }

    /// Allocate memory from heap
    pub fn allocate(self: *Heap, size: u32, alignment: u32) !u32 {
        const aligned_addr = (self.base_addr + self.used + alignment - 1) & ~(alignment - 1);
        const total_needed = (aligned_addr - self.base_addr) + size;

        if (total_needed > self.size) {
            return error.HeapExhausted;
        }

        self.used = total_needed;
        return aligned_addr;
    }

    pub fn getUsedMemory(self: *const Heap) u32 {
        return self.used;
    }

    pub fn getAvailableMemory(self: *const Heap) u32 {
        return self.size - self.used;
    }
};
