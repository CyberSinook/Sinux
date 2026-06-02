const std = @import("std");
const drivers = @import("../drivers/mod.zig");

pub const ServiceType = enum {
    game,
    shell,
    filesystem,
    network,
};

pub const ServiceState = enum {
    stopped,
    running,
    paused,
};

pub const Service = struct {
    id: u32,
    name: []const u8,
    service_type: ServiceType,
    start: ?*const fn () anyerror!void = null,
    stop: ?*const fn () anyerror!void = null,
    state: ServiceState,
};

pub const InitSystem = struct {
    services: std.ArrayList(Service),
    default_service: ?u32 = null,
    allocator: std.mem.Allocator,

    pub fn create(allocator: std.mem.Allocator) !InitSystem {
        return InitSystem{
            .services = std.ArrayList(Service).init(allocator),
            .allocator = allocator,
        };
    }

    pub fn destroy(self: *InitSystem) void {
        self.services.deinit();
    }

    pub fn registerService(self: *InitSystem, allocator: std.mem.Allocator, service: Service) !void {
        _ = allocator;
        try self.services.append(service);

        var buf: [128]u8 = undefined;
        const msg = try std.fmt.bufPrint(&buf, "[INIT] Registered service: {s}\n", .{service.name});
        try drivers.serial.write(msg);
    }

    pub fn setDefaultService(self: *InitSystem, service_id: u32) !void {
        if (service_id < self.services.items.len) {
            self.default_service = service_id;
            try drivers.serial.print("[INIT] Set default service\n", .{});
        } else {
            return error.InvalidServiceId;
        }
    }

    pub fn runDefaultService(self: *InitSystem) !void {
        if (self.default_service) |id| {
            if (id < self.services.items.len) {
                var service = &self.services.items[id];
                if (service.start) |start_fn| {
                    var buf: [128]u8 = undefined;
                    const msg = try std.fmt.bufPrint(&buf, "[INIT] Running service: {s}\n", .{service.name});
                    try drivers.serial.write(msg);

                    service.state = .running;
                    try start_fn();
                    service.state = .stopped;
                }
            }
        } else {
            try drivers.serial.print("[INIT] No default service\n", .{});
        }
    }
};
