const std = @import("std");

pub const pmm = @import("pmm.zig");
pub const vmm = @import("vmm.zig");
pub const heap = @import("heap.zig");

pub const MemoryManager = struct {
    pmm_instance: pmm.PhysicalMemoryManager,
    vmm_instance: vmm.VirtualMemoryManager,
    heap_instance: heap.Heap,
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator) !MemoryManager {
        return MemoryManager{
            .pmm_instance = try pmm.PhysicalMemoryManager.init(allocator),
            .vmm_instance = try vmm.VirtualMemoryManager.init(allocator),
            .heap_instance = try heap.Heap.init(allocator),
            .allocator = allocator,
        };
    }

    pub fn deinit(self: *MemoryManager) void {
        self.pmm_instance.deinit();
        self.vmm_instance.deinit();
        self.heap_instance.deinit();
    }
};
