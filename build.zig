const std = @import("std");

const FLAGS = [_][]const u8{
    "-std=c++20",
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    var user_config = b.addConfigHeader(.{
        .style = .{ .cmake = b.path("config.h.in") },
        .include_path = "config.h",
    }, .{});

    const cimgui_dep = b.dependency("cimgui", .{
        .target = target,
        .optimize = optimize,
    });
    // inject the cimgui header search path into the sokol C library compile step
    const cimgui_root = cimgui_dep.namedWriteFiles("cimgui").getDirectory();
    const glfw_dep = b.dependency("glfw", .{
        .target = target,
        .optimize = optimize,
    });

    const dxmath_dep = b.dependency("directxmath", .{});
    const plog_dep = b.dependency("plog", .{});
    const watcher_dep = b.dependency("simplefilewatcher", .{});
    const glew_dep = b.dependency("glew", .{});
    const grapho_dep = b.dependency("grapho", .{});
    const gltfjson_dep = b.dependency("gltfjson", .{});
    const lua_dep = b.dependency("lua", .{});

    const exe = b.addExecutable(.{
        .name = "vrmeditor",
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath(user_config.getOutput().dirname());
    exe.linkLibCpp();
    exe.addCSourceFiles(.{
        .root = b.path("src/vrmeditor/"),
        .files = &.{
            "main.cpp",
            "platform.cpp",
            "app.cpp",
            "luahost.cpp",
            "filewatcher.cpp",
            "fbx_loader.cpp",
            "scene_state.cpp",
            // "docks/asset_view.cpp",
            // "docks/gui.cpp",
            // "docks/dockspace.cpp",
            // "docks/imlogger.cpp",
            // "docks/vrm_gui.cpp",
            // "docks/export_dock.cpp",
            // "docks/hierarchy_gui.cpp",
            // "docks/humanoid_dock.cpp",
            // "docks/springbone_gui.cpp",
            // "view/scene_preview.cpp",
            // "view/im_fbo.cpp",
            // "view/overlay.cpp",
            // "view/lighting.cpp",
            // "view/gl3renderer_gui.cpp",
            // "view/animation_view.cpp",
            // "view/mesh_gui.cpp",
            // "humanpose/humanpose_stream.cpp",
            // "humanpose/bvhnode.cpp",
            // "humanpose/udpnode.cpp",
        },
        .flags = &FLAGS,
    });
    exe.addIncludePath(b.path(""));
    exe.addIncludePath(b.path("src/vrmeditor"));
    exe.addIncludePath(b.path("subprojects/ImGuiColorTextEdit"));
    exe.addIncludePath(plog_dep.path("include"));
    exe.addIncludePath(glew_dep.path("include"));
    exe.addIncludePath(grapho_dep.path("src"));
    exe.addIncludePath(glfw_dep.builder.dependency("glfw", .{}).path("include"));
    exe.addIncludePath(gltfjson_dep.path("include"));
    exe.addIncludePath(watcher_dep.path("trunk/include"));
    exe.addIncludePath(dxmath_dep.path("Inc"));
    exe.addIncludePath(lua_dep.path("src"));
    exe.addCSourceFiles(.{
        .root = watcher_dep.path("trunk/source"),
        .files = &.{
            "FileWatcher.cpp",
            "FileWatcherWin32.cpp",
        },
    });
    exe.addIncludePath(b.path("libvrm"));
    exe.addIncludePath(b.path("boneskin"));
    exe.addIncludePath(b.path("glrenderer"));
    exe.addIncludePath(b.path("subprojects/ufbx"));
    exe.addIncludePath(cimgui_root.path(b, "imgui"));
    exe.addCSourceFile(.{ .file = cimgui_dep.path("custom_button_behaviour.cpp") });

    b.installArtifact(exe);
}
