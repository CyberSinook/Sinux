const std = @import("std");
const builtin = @import("builtin");

/// BSD Bootloader info structure passed by bootloader
pub const BSDBootInfo = extern struct {
    magic: u32,                    // 0x57494E42 ('WINB')
    boot_device: u8,              // Boot device number
    boot_flags: u8,               // Boot flags
    kernel_entry: u32,            // Kernel entry point
    lower_memory: u16,            // Lower memory (KB)
    upper_memory: u16,            // Upper memory (KB)
    extended_memory: u16,         // Extended memory (KB)
};

/// BSD-compatible kernel entry point
pub export fn kernelMainBSD(boot_info: *BSDBootInfo) noreturn {
    const drivers = @import("../drivers/mod.zig");
    const mm = @import("../mm/mod.zig");
    const arch = @import("../arch/x86/mod.zig");
    const sys = @import("../sys/mod.zig");

    // Verify BSD boot magic
    if (boot_info.magic != 0x57494E42) {
        while (true) asm volatile ("hlt");
    }

    var gpa = std.heap.GeneralPurposeAllocator(.{
        .safety = true,
    }){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    // Initialize drivers first (serial output)
    drivers.serial.init() catch {};
    drivers.pic.init() catch {};

    // Print boot message
    drivers.serial.print("\n", .{}) catch {};
    drivers.serial.print("╔════════════════════════════════════════╗\n", .{}) catch {};
    drivers.serial.print("║  SINUX OS (Zig Edition) - BSD Boot    ║\n", .{}) catch {};
    drivers.serial.print("╚════════════════════════════════════════╝\n", .{}) catch {};

    drivers.serial.print("\n[BOOT] BSD Bootloader Info:\n", .{}) catch {};
    drivers.serial.print("  Boot Device: 0x{X:0>2}\n", .{boot_info.boot_device}) catch {};
    drivers.serial.print("  Boot Flags: 0x{X:0>2}\n", .{boot_info.boot_flags}) catch {};
    drivers.serial.print("  Kernel Entry: 0x{X:0>8}\n", .{boot_info.kernel_entry}) catch {};
    drivers.serial.print("  Lower Memory: {} KB\n", .{boot_info.lower_memory}) catch {};
    drivers.serial.print("  Upper Memory: {} KB\n", .{boot_info.upper_memory}) catch {};
    drivers.serial.print("  Extended Memory: {} KB\n", .{boot_info.extended_memory}) catch {};

    // Initialize memory management
    drivers.serial.print("\n[INIT] Initializing memory management...\n", .{}) catch {};
    mm.pmm.init(allocator) catch {};
    mm.vmm.init(allocator) catch {};
    mm.heap.init(allocator) catch {};

    // Initialize CPU
    drivers.serial.print("[INIT] Initializing CPU...\n", .{}) catch {};
    arch.cpu.gdt.init() catch {};
    arch.cpu.idt.init(allocator) catch {};
    arch.cpu.interrupts.init() catch {};

    // Initialize init system
    drivers.serial.print("[INIT] Initializing system services...\n", .{}) catch {};
    var init_system = sys.init_mod.InitSystem.create(allocator) catch {
        drivers.serial.print("ERROR: Failed to create init system\n", .{}) catch {};
        while (true) asm volatile ("hlt");
    };
    defer init_system.destroy();

    // Register services
    const games = @import("../games/mod.zig");
    const pacman_service = sys.init_mod.Service{
        .id = 0,
        .name = "Pacman Game",
        .service_type = .game,
        .start = games.pacman.serviceStart,
        .stop = games.pacman.serviceStop,
        .state = .stopped,
    };
    init_system.registerService(allocator, pacman_service) catch {};

    // Set default service
    init_system.setDefaultService(0) catch {};

    // Run default service
    init_system.runDefaultService() catch {};

    drivers.serial.print("\n[BOOT] System shutdown\n", .{}) catch {};

    while (true) asm volatile ("hlt");
}
