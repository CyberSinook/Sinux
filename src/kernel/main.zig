const std = @import("std");
const builtin = @import("builtin");

// Import all kernel modules
pub const mm = @import("kernel/mm/mod.zig");
pub const arch = @import("kernel/arch/x86/mod.zig");
pub const drivers = @import("kernel/drivers/mod.zig");
pub const sys = @import("kernel/sys/mod.zig");
pub const games = @import("kernel/games/mod.zig");
pub const boot_bsd = @import("kernel/boot_bsd.zig");

/// Multiboot info structure
pub const MultibootInfo = extern struct {
    flags: u32,
    mem_lower: u32,
    mem_upper: u32,
    boot_device: u32,
    cmdline: u32,
    mods_count: u32,
    mods_addr: u32,
    syms: [4]u32,
    mmap_length: u32,
    mmap_addr: u32,
    drives_length: u32,
    drives_addr: u32,
    config_table: u32,
    boot_loader_name: u32,
    apm_table: u32,
    vbe_control_info: u32,
    vbe_mode_info: u32,
    vbe_mode: u16,
    vbe_interface_seg: u16,
    vbe_interface_off: u16,
    vbe_interface_len: u16,
};

var gpa = std.heap.GeneralPurposeAllocator(.{
    .safety = true,
}){};

pub fn main() !void {
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    // Initialize memory management
    try mm.pmm.init(allocator);
    try mm.vmm.init(allocator);
    try mm.heap.init(allocator);

    // Initialize CPU
    try arch.cpu.gdt.init();
    try arch.cpu.idt.init(allocator);
    try arch.cpu.interrupts.init();

    // Initialize drivers
    try drivers.serial.init();
    try drivers.pic.init();

    // Print boot message
    try drivers.serial.print("\n=== SINUX OS (Zig Edition) ===\n", .{});
    try drivers.serial.print("Initializing system...\n", .{});

    // Initialize init system
    var init_system = try sys.init_mod.InitSystem.create(allocator);
    defer init_system.destroy();

    // Register Pacman game service
    const pacman_service = sys.init_mod.Service{
        .id = 0,
        .name = "Pacman Game",
        .service_type = .game,
        .start = games.pacman.serviceStart,
        .stop = games.pacman.serviceStop,
        .state = .stopped,
    };
    try init_system.registerService(allocator, pacman_service);

    // Set Pacman as default service
    try init_system.setDefaultService(0);

    // Run default service
    try init_system.runDefaultService();

    // Shutdown
    try drivers.serial.print("\n[INIT] System shutdown\n", .{});

    while (true) {
        asm volatile ("hlt");
    }
}

/// Kernel entry point (called from assembly)
pub export fn kernelMain(multiboot_info: *MultibootInfo) noreturn {
    main() catch |err| {
        _ = drivers.serial.print("Fatal error: {any}\n", .{err}) catch {};
    };

    while (true) {
        asm volatile ("hlt");
    }
}
