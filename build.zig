const std = @import("std");
// const zcc = @import("compile_commands");

const FLAGS_CPP = [_][]const u8{
    "-std=c++20",
};

const FLAGS = [_][]const u8{
    "-Wno-defaulted-function-deleted",
    "-DNOMINMAX",
    "-D_UTF8",
    "-DPLOG_CHAR_IS_UTF8=1",
    "-DIMGUI_USE_WCHAR32=1",
    "-DImDrawIdx=unsigned int",
    // "-DIMGUI_ENABLE_FREETYPE",
    "-DIMGUI_DEFINE_MATH_OPERATORS",
    "-DUSE_BOOKMARK=1",
    "-DBASISD_SUPPORT_FXT1=0",
    "-DBASISD_SUPPORT_KTX2=1",
    "-DBASISD_SUPPORT_KTX2_ZSTD=0",
    "-DBASISU_NO_ITERATOR_DEBUG_LEVEL",
    "-DBASISU_SUPPORT_OPENCL=0",
    // "-DBASISU_SUPPORT_SSE=1",
    "-DKTX_API=__declspec(dllexport)",
    "-DKTX_FEATURE_KTX1",
    "-DKTX_FEATURE_KTX2",
    "-DKTX_FEATURE_WRITE",
    "-DLIBKTX",
    "-Dktx_EXPORTS",
    "-DGLEW_STATIC",
    "-D_WIN32",
    "-D_GLFW_WIN32",
};

const FLAGS_WITH_CPP = FLAGS ++ FLAGS_CPP;

pub fn build(b: *std.Build) void {
    // var targets = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const user_config = b.addConfigHeader(.{
        .style = .{ .cmake = b.path("config.h.in") },
        .include_path = "config.h",
    }, .{
        .PACKAGE = "\"vrmeditor\"",
        .PACKAGE_VERSION = "\"0.0.0\"",
    });

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
    const glew_dep = b.dependency("glew", .{
        .target = target,
        .optimize = optimize,
    });
    const grapho_dep = b.dependency("grapho", .{});
    const gltfjson_dep = b.dependency("gltfjson", .{});
    const lua_dep = b.dependency("lua", .{});
    const iconfont_dep = b.dependency("iconfont", .{});
    const cuber_dep = b.dependency("cuber", .{});
    const ktx_dep = b.dependency("ktx", .{});
    const stb_dep = b.dependency("stb", .{
        .target = target,
        .optimize = optimize,
    });

    const exe = b.addExecutable(.{
        .name = "vrmeditor",
        .target = target,
        .optimize = optimize,
    });
    const install = b.addInstallArtifact(exe, .{});
    b.getInstallStep().dependOn(&install.step);
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
            "docks/gui.cpp",
            "docks/dockspace.cpp",
            "docks/imlogger.cpp",
            "docks/vrm_gui.cpp",
            "docks/export_dock.cpp",
            "docks/hierarchy_gui.cpp",
            "docks/humanoid_dock.cpp",
            "docks/springbone_gui.cpp",
            "view/scene_preview.cpp",
            "view/im_fbo.cpp",
            "view/overlay.cpp",
            "view/lighting.cpp",
            "view/gl3renderer_gui.cpp",
            "view/animation_view.cpp",
            "view/mesh_gui.cpp",
            "humanpose/humanpose_stream.cpp",
            "humanpose/bvhnode.cpp",
            //
            "fs_util_win32.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.addCSourceFiles(.{
        .root = b.path("recti"),
        .files = &.{
            "vec4.cpp",
            "mat4.cpp",
            "camera_mouse.cpp",
            "style.cpp",
            "drawcommand.cpp",
            "tripod.cpp",
            "handle/translation_gizmo.cpp",
            "handle/translation_drag.cpp",
            "handle/rotation_gizmo.cpp",
            "handle/rotation_drag.cpp",
            "handle/scale_gizmo.cpp",
            "handle/scale_drag.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.addCSourceFiles(.{
        .root = b.path("glrenderer"),
        .files = &.{
            "glr/gl3renderer.cpp",
            "glr/shader_source.cpp",
            "glr/material_three_vrm0.cpp",
            "glr/material_three_vrm1.cpp",
            "glr/material_pbr_khronos.cpp",
            "glr/gizmo.cpp",
            "glr/rendertarget.cpp",
            "glr/scene_renderer.cpp",
            "glr/material.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.addCSourceFiles(.{
        .root = b.path("libvrm"),
        .files = &.{
            "vrm/gltfroot.cpp",
            "vrm/node.cpp",
            "vrm/image.cpp",
            "vrm/importer.cpp",
            "vrm/runtime_scene.cpp",
            "vrm/animation.cpp",
            // "vrm/timeline.cpp",
            "vrm/spring_bone.cpp",
            "vrm/spring_collision.cpp",
            "vrm/runtime_springjoint.cpp",
            "vrm/network/srht_update.cpp",
            // "vrm/network/srht_sender.cpp",
            "vrm/bvh/bvh.cpp",
            "vrm/bvh/bvhframe.cpp",
            "vrm/bvh/bvhscene.cpp",
            "vrm/humanoid/humanskeleton.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.addCSourceFiles(.{
        .root = b.path("boneskin"),
        .files = &.{
            "boneskin/meshdeformer.cpp",
            "boneskin/deformed_mesh.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.addCSourceFiles(.{
        .root = b.path("jsongui"),
        .files = &.{
            "json_gui.cpp",
            "jsonschema/json_prop.cpp",
            "jsonschema/gltf.cpp",
            "jsonschema/vrm0.cpp",
            "jsonschema/vrm1.cpp",
            "jsonschema/extensions.cpp",
            "json_widgets.cpp",
            "jsonschema/tag.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.addCSourceFiles(.{
        .root = b.path("subprojects"),
        .files = &.{
            "ImGuiColorTextEdit/TextEditor.cpp",
            "ImGuiFileDialog/ImGuiFileDialog.cpp",
            "imnodes/imnodes.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.addCSourceFiles(.{
        .root = b.path("subprojects"),
        .files = &.{
            "ufbx/ufbx.c",
        },
    });
    exe.addIncludePath(b.path(""));
    exe.addIncludePath(b.path("src/vrmeditor"));
    exe.addIncludePath(b.path("recti"));
    exe.addIncludePath(b.path("jsongui"));
    exe.addIncludePath(b.path("subprojects/ImGuiColorTextEdit"));
    exe.addIncludePath(b.path("subprojects/ImGuiFileDialog"));
    exe.addIncludePath(b.path("subprojects/imnodes/"));
    exe.addIncludePath(iconfont_dep.path(""));

    exe.addIncludePath(b.path("subprojects/packagefiles/ktx"));
    exe.addIncludePath(ktx_dep.path("include"));
    exe.addIncludePath(ktx_dep.path("include"));
    exe.addIncludePath(ktx_dep.path("lib/basisu/transcoder"));
    exe.addIncludePath(ktx_dep.path("lib/basisu/zstd"));
    exe.addIncludePath(ktx_dep.path("other_include"));
    exe.addIncludePath(ktx_dep.path("utils"));
    exe.addIncludePath(ktx_dep.path("lib/dfdutils"));
    exe.addIncludePath(ktx_dep.path("lib/basisu"));
    exe.addIncludePath(ktx_dep.path("lib/astc-encoder/Source"));
    exe.addCSourceFiles(.{
        .root = ktx_dep.path(""),
        .files = &.{
            "lib/basis_transcode.cpp",
            "lib/basisu/zstd/zstd.c",
            "lib/checkheader.c",
            "lib/dfdutils/colourspaces.c",
            "lib/dfdutils/createdfd.c",
            "lib/dfdutils/interpretdfd.c",
            "lib/dfdutils/printdfd.c",
            "lib/dfdutils/queries.c",
            "lib/dfdutils/vk2dfd.c",
            "lib/etcdec.cxx",
            "lib/etcunpack.cxx",
            "lib/filestream.c",
            "lib/hashlist.c",
            "lib/info.c",
            "lib/memstream.c",
            "lib/miniz_wrapper.cpp",
            "lib/strings.c",
            "lib/swap.c",
            "lib/texture.c",
            "lib/texture2.c",
            "lib/vkformat_check.c",
            "lib/vkformat_str.c",

            "lib/basisu/encoder/basisu_backend.cpp",
            "lib/basisu/encoder/basisu_basis_file.cpp",
            "lib/basisu/encoder/basisu_bc7enc.cpp",
            "lib/basisu/encoder/basisu_comp.cpp",
            "lib/basisu/encoder/basisu_enc.cpp",
            "lib/basisu/encoder/basisu_etc.cpp",
            "lib/basisu/encoder/basisu_frontend.cpp",
            "lib/basisu/encoder/basisu_gpu_texture.cpp",
            "lib/basisu/encoder/basisu_kernels_sse.cpp",
            "lib/basisu/encoder/basisu_opencl.cpp",
            "lib/basisu/encoder/basisu_pvrtc1_4.cpp",
            "lib/basisu/encoder/basisu_resample_filters.cpp",
            "lib/basisu/encoder/basisu_resampler.cpp",
            "lib/basisu/encoder/basisu_ssim.cpp",
            "lib/basisu/encoder/basisu_uastc_enc.cpp",
            "lib/gl_funcs.c",
            "lib/glloader.c",
            // "lib/basisu/basisu_tool.cpp",
            "lib/basisu/encoder/basisu_backend.cpp",
            "lib/basisu/encoder/basisu_basis_file.cpp",
            "lib/basisu/encoder/basisu_bc7enc.cpp",
            "lib/basisu/encoder/basisu_comp.cpp",
            "lib/basisu/encoder/basisu_enc.cpp",
            "lib/basisu/encoder/basisu_etc.cpp",
            "lib/basisu/encoder/basisu_frontend.cpp",
            "lib/basisu/encoder/basisu_gpu_texture.cpp",
            "lib/basisu/encoder/basisu_kernels_sse.cpp",
            "lib/basisu/encoder/basisu_opencl.cpp",
            "lib/basisu/encoder/basisu_pvrtc1_4.cpp",
            "lib/basisu/encoder/basisu_resample_filters.cpp",
            "lib/basisu/encoder/basisu_resampler.cpp",
            "lib/basisu/encoder/basisu_ssim.cpp",
            "lib/basisu/encoder/basisu_uastc_enc.cpp",
            "lib/basisu/encoder/jpgd.cpp",
            "lib/basisu/encoder/pvpngreader.cpp",
            "lib/basisu/transcoder/basisu_transcoder.cpp",
            "lib/astc-encoder/Source/astcenc_averages_and_directions.cpp",
            "lib/astc-encoder/Source/astcenc_block_sizes.cpp",
            "lib/astc-encoder/Source/astcenc_color_quantize.cpp",
            "lib/astc-encoder/Source/astcenc_color_unquantize.cpp",
            "lib/astc-encoder/Source/astcenc_compress_symbolic.cpp",
            "lib/astc-encoder/Source/astcenc_compute_variance.cpp",
            "lib/astc-encoder/Source/astcenc_decompress_symbolic.cpp",
            "lib/astc-encoder/Source/astcenc_diagnostic_trace.cpp",
            "lib/astc-encoder/Source/astcenc_entry.cpp",
            "lib/astc-encoder/Source/astcenc_find_best_partitioning.cpp",
            "lib/astc-encoder/Source/astcenc_ideal_endpoints_and_weights.cpp",
            "lib/astc-encoder/Source/astcenc_image.cpp",
            "lib/astc-encoder/Source/astcenc_integer_sequence.cpp",
            "lib/astc-encoder/Source/astcenc_mathlib.cpp",
            "lib/astc-encoder/Source/astcenc_mathlib_softfloat.cpp",
            "lib/astc-encoder/Source/astcenc_partition_tables.cpp",
            "lib/astc-encoder/Source/astcenc_percentile_tables.cpp",
            "lib/astc-encoder/Source/astcenc_pick_best_endpoint_format.cpp",
            "lib/astc-encoder/Source/astcenc_quantization.cpp",
            "lib/astc-encoder/Source/astcenc_symbolic_physical.cpp",
            "lib/astc-encoder/Source/astcenc_weight_align.cpp",
            "lib/astc-encoder/Source/astcenc_weight_quant_xfer_tables.cpp",
            "lib/basisu/transcoder/basisu_transcoder.cpp",
            "lib/texture1.c",
            "lib/basis_encode.cpp",
            "lib/astc_encode.cpp",
            "lib/writer1.c",
            "lib/writer2.c",
        },
        .flags = &FLAGS,
    });
    exe.addIncludePath(plog_dep.path("include"));
    exe.addIncludePath(glew_dep.builder.dependency("glew", .{}).path("include"));
    exe.linkLibrary(glew_dep.artifact("glew"));
    exe.addIncludePath(cuber_dep.path("cuber/include"));
    exe.addCSourceFiles(.{
        .root = cuber_dep.path("cuber"),
        .files = &.{
            "src/gl3/GlCubeRenderer.cpp",
            "src/gl3/GlLineRenderer.cpp",
            "src/mesh.cpp",
            // "src/dx/DxCubeRenderer.cpp",
            // "src/dx/DxCubeRendererImpl.cpp",
            // "src/dx/DxCubeStereoRenderer.cpp",
            // "src/dx/DxLineRenderer.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.addIncludePath(grapho_dep.path("src"));
    exe.addIncludePath(glfw_dep.builder.dependency("glfw", .{}).path("include"));
    exe.linkLibrary(glfw_dep.artifact("glfw"));
    exe.addIncludePath(gltfjson_dep.path("include"));
    exe.addIncludePath(watcher_dep.path("trunk/include"));
    exe.addIncludePath(dxmath_dep.path("Inc"));
    exe.addIncludePath(lua_dep.path("src"));
    exe.addCSourceFiles(.{
        .root = lua_dep.path("src"),
        .files = &.{ "lapi.c", "lcode.c", "lctype.c", "ldebug.c", "ldo.c", "ldump.c", "lfunc.c", "lgc.c", "llex.c", "lmem.c", "lobject.c", "lopcodes.c", "lparser.c", "lstate.c", "lstring.c", "ltable.c", "ltm.c", "lundump.c", "lvm.c", "lzio.c", "lauxlib.c", "lbaselib.c", "lcorolib.c", "ldblib.c", "liolib.c", "lmathlib.c", "loadlib.c", "loslib.c", "lstrlib.c", "ltablib.c", "lutf8lib.c", "linit.c" },
    });
    exe.addCSourceFiles(.{
        .root = watcher_dep.path("trunk/source"),
        .files = &.{
            "FileWatcher.cpp",
            "FileWatcherWin32.cpp",
        },
    });
    exe.addIncludePath(stb_dep.path(""));
    exe.linkLibrary(stb_dep.artifact("stb"));
    exe.addIncludePath(b.path("libvrm"));
    exe.addIncludePath(b.path("boneskin"));
    exe.addIncludePath(b.path("glrenderer"));
    exe.addIncludePath(b.path("subprojects/ufbx"));
    exe.addIncludePath(cimgui_root.path(b, "imgui"));
    exe.addIncludePath(cimgui_root.path(b, "imgui/backends"));
    exe.addCSourceFiles(.{
        .root = cimgui_root.path(b, "imgui"),
        .files = &.{
            "imgui.cpp",
            "imgui_demo.cpp",
            "imgui_draw.cpp",
            "imgui_tables.cpp",
            "imgui_widgets.cpp",
            "backends/imgui_impl_glfw.cpp",
            "backends/imgui_impl_opengl3.cpp",
            "misc/cpp/imgui_stdlib.cpp",
            // "misc/freetype/imgui_freetype.cpp",
        },
        .flags = &FLAGS,
    });

    exe.addCSourceFile(.{ .file = cimgui_dep.path("custom_button_behaviour.cpp") });
    exe.addCSourceFiles(.{
        .root = grapho_dep.path("src"),
        .files = &.{
            "grapho/euclidean_transform.cpp",
            "grapho/vars.cpp",
            "grapho/camera/camera.cpp",
            "grapho/camera/ray.cpp",
            "grapho/gl3/vao.cpp",
            "grapho/gl3/texture.cpp",
            "grapho/gl3/shader.cpp",
            "grapho/gl3/cuberenderer.cpp",
            "grapho/gl3/fbo.cpp",
            "grapho/gl3/error_check.cpp",
        },
        .flags = &FLAGS_WITH_CPP,
    });
    exe.linkSystemLibrary("OpenGL32");
    exe.linkSystemLibrary("GDI32");

    // targets.append(exe) catch @panic("append");

    const install_dir = b.addInstallDirectory(.{
        .source_dir = b.path("glrenderer/shaders"),
        .install_dir = .{ .prefix = void{} },
        .install_subdir = "bin/shaders",
    });
    b.getInstallStep().dependOn(&install_dir.step);

    //
    // sokol-zig version
    //
    const sokol_dep = b.dependency("sokol", .{
        .target = target,
        .optimize = optimize,
        .with_sokol_imgui = true,
        .gl = true,
    });
    sokol_dep.artifact("sokol_clib").addIncludePath(cimgui_root.path(b, "imgui"));
    sokol_dep.artifact("sokol_clib").addIncludePath(cimgui_root);
    sokol_dep.artifact("sokol_clib").addCSourceFile(.{ .file = b.path("deps/cimgui/custom_button_behaviour.cpp") });
    const z = b.addExecutable(.{
        .name = "vrmeditorz",
        .target = target,
        .optimize = optimize,
        .root_source_file = b.path("main.zig"),
    });
    const z_install = b.addInstallArtifact(z, .{});
    b.getInstallStep().dependOn(&z_install.step);
    z.root_module.addImport("sokol", sokol_dep.module("sokol"));
    z.root_module.addImport("cimgui", cimgui_dep.module("cimgui"));

    const run = b.addRunArtifact(z);
    if (b.args) |args| {
        run.addArgs(args);
    }
    run.setCwd(b.path("zig-out/bin"));
    run.step.dependOn(&z_install.step);
    b.step("run", "run vrmeditor zig version").dependOn(&run.step);

    // zcc.createStep(b, "cdb", targets.toOwnedSlice() catch @panic("OOM"));
}
