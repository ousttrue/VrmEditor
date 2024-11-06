const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const lib = b.addStaticLibrary(.{
        .name = "glfw",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    b.installArtifact(lib);

    const glfw_dep = b.dependency("glfw", .{});
    lib.addIncludePath(glfw_dep.path("include"));
    lib.addCSourceFiles(.{
        .root = glfw_dep.path("src"),
        .files = &.{
            "context.c",
            "init.c",
            "input.c",
            "monitor.c",
            "platform.c",
            "vulkan.c",
            "window.c",
            "egl_context.c",
            "osmesa_context.c",
            "null_init.c",
            "null_monitor.c",
            "null_window.c",
            "null_joystick.c",
            // win32
            "win32_module.c",
            "win32_time.c",
            "win32_thread.c",
            "win32_init.c",
            "win32_joystick.c",
            "win32_monitor.c",
            "win32_window.c",
            "wgl_context.c",
        },
        .flags = &.{
            "-D_GLFW_WIN32",
        },
    });
}
