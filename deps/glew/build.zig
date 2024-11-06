const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const lib = b.addStaticLibrary(.{
        .name = "glew",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    b.installArtifact(lib);

    const glew_dep = b.dependency("glew", .{});
    lib.addIncludePath(glew_dep.path("include"));
    lib.addCSourceFiles(.{
        .root = glew_dep.path("src"),
        .files = &.{
            "glew.c",
        },
        .flags = &.{"-DGLEW_STATIC"},
    });
}
