const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const lib = b.addStaticLibrary(.{
        .target = target,
        .optimize = optimize,
        .name = "stb",
        .link_libc = true,
    });
    b.installArtifact(lib);

    lib.addCSourceFiles(.{
        .files = &.{
            "stb_image.c",
        },
    });
}
