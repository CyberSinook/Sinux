const std = @import("std");
const builtin = @import("builtin");

/// Build script for Sinux OS in Zig
pub fn build(b: *std.Build) void {
    const target = b.resolveTargetQuery(.{
        .cpu_arch = .x86,
        .os_tag = .freestanding,
        .abi = .none,
    });

    const optimize = b.standardOptimizeOption(.{});

    // Create kernel executable
    const kernel = b.addExecutable(.{
        .name = "kernel.elf",
        .root_source_file = b.path("src/kernel/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    kernel.setLinkerScript(b.path("src/linker.ld"));
    kernel.root_module.strip = true;

    // Add bootloader object file
    const bootloader_obj = b.addObject(.{
        .name = "bootloader",
        .target = target,
        .optimize = optimize,
    });
    bootloader_obj.addAssemblyFile(b.path("src/boot/multiboot.s"));
    kernel.addObject(bootloader_obj);

    b.installArtifact(kernel);

    // Create ISO image step
    const iso_step = b.step("iso", "Build ISO image");
    const iso_cmd = b.addSystemCommand(&.{
        "grub-mkrescue",
        "-o",
        b.getInstallPath(.bin, "sinux.iso"),
        b.getInstallPath(.bin, "."),
    });
    iso_step.dependOn(&iso_cmd.step);
    iso_step.dependOn(b.getInstallStep());
}
