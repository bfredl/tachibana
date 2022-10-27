const std = @import("std");
const Builder = std.build.Builder;
const ScanProtocolsStep = @import("deps/zig-wayland/build.zig").ScanProtocolsStep;

pub fn build(b: *Builder) void {
    const mode = b.standardReleaseOptions();

    const scanner = ScanProtocolsStep.create(b);
    scanner.addSystemProtocol("stable/xdg-shell/xdg-shell.xml");

    scanner.generate("wl_compositor", 1);
    scanner.generate("wl_shm", 1);
    scanner.generate("xdg_wm_base", 1);

    const wayland = std.build.Pkg{ .name = "wayland", .source = .{ .generated = &scanner.result } };

    const exe = b.addExecutable("wlshell", "src/main.zig");
    exe.setBuildMode(mode);

    exe.step.dependOn(&scanner.step);
    exe.addPackage(wayland);
    exe.linkLibC();
    exe.linkSystemLibrary("wayland-client");

    // TODO: remove when https://github.com/ziglang/zig/issues/131 is implemented
    scanner.addCSource(exe);

    exe.install();

    const activate = b.step("activate", "do the thing");
    const run = exe.run();
    run.step.dependOn(b.getInstallStep());
    activate.dependOn(&run.step);
}
