const std = @import("std");

/// Virtual Memory Manager for Sinux
pub const VirtualMemoryManager = struct {
    page_tables: std.ArrayList(u32),
    allocator: std.mem.Allocator,
    is_enabled: bool,

    const PAGE_SIZE = 4096;
    const PAGE_TABLE_SIZE = 1024;

    pub fn init(allocator: std.mem.Allocator) !VirtualMemoryManager {
        var page_tables = std.ArrayList(u32).init(allocator);
        try page_tables.resize(PAGE_TABLE_SIZE);
        @memset(page_tables.items, 0);

        return VirtualMemoryManager{
            .page_tables = page_tables,
            .allocator = allocator,
            .is_enabled = false,
        };
    }

    pub fn deinit(self: *VirtualMemoryManager) void {
        self.page_tables.deinit();
    }

    /// Map virtual address to physical address
    pub fn mapPage(self: *VirtualMemoryManager, virt_addr: u32, phys_addr: u32, flags: u32) !void {
        const page_idx = virt_addr / PAGE_SIZE;
        if (page_idx < self.page_tables.items.len) {
            self.page_tables.items[page_idx] = phys_addr | flags;
        } else {
            return error.PageTableFull;
        }
    }

    /// Unmap virtual address
    pub fn unmapPage(self: *VirtualMemoryManager, virt_addr: u32) void {
        const page_idx = virt_addr / PAGE_SIZE;
        if (page_idx < self.page_tables.items.len) {
            self.page_tables.items[page_idx] = 0;
        }
    }

    /// Enable paging
    pub fn enablePaging(self: *VirtualMemoryManager) void {
        self.is_enabled = true;
        // Load page table and enable paging via inline asm
        asm volatile (
            \\  mov cr3, %[pt]
            \\  mov eax, cr0
            \\  or eax, 0x80000000
            \\  mov cr0, eax
            : [pt] "r" (@intFromPtr(self.page_tables.items.ptr))
        );
    }
};
