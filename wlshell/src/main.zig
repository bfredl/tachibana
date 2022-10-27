const std = @import("std");
const os = std.os;
const print = std.debug.print;

const wayland = @import("wayland");
const wl = wayland.client.wl;
const xdg = wayland.client.xdg;

const Shell = struct {};

pub fn main() !void {
    const display = try wl.Display.connect(null);
    const registry = try display.getRegistry();

    var shell: Shell = .{};

    registry.setListener(*Shell, registryListener, &shell);
    if (display.roundtrip() != .SUCCESS) return error.RoundtripFailed;
}

fn registryListener(registry: *wl.Registry, event: wl.Registry.Event, shell: *Shell) void {
    _ = registry;
    _ = shell;
    switch (event) {
        .global => |global| {
            print("registered global {s}\n", .{std.mem.span(global.interface)});
        },
        .global_remove => {},
    }
}
