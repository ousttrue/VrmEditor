srcs = [
    'main.cpp',
    'platform.cpp',
    'app.cpp',
    'luahost.cpp',
    'assetdir.cpp',
    'filewatcher.cpp',
    'glr/gl3renderer.cpp',
    'glr/gl3renderer_gui.cpp',
    'glr/cuber.cpp',
    'glr/scene_preview.cpp',
    'glr/rendertarget.cpp',
    'glr/line_gizmo.cpp',
    'glr/overlay.cpp',
    'glr/rendering_env.cpp',
    'glr/shader_source.cpp',
    'glr/error_check.cpp',
    'glr/material_three_vrm.cpp',
    'docks/gui.cpp',
    'docks/imlogger.cpp',
    'docks/vrm_dock.cpp',
    'docks/export_dock.cpp',
    'docks/gltfjson_gui.cpp',
    'docks/type_gui.cpp',
    'docks/type_gui_vrm0.cpp',
    'docks/json_gui.cpp',
    'docks/json_gui_factory.cpp',
    'docks/json_gui_labelcache.cpp',
    'docks/im_widgets.cpp',
    'humanpose/humanpose_stream.cpp',
    'humanpose/udp_receiver.cpp',
    'humanpose/bvhnode.cpp',
    'humanpose/udpnode.cpp',
]

if host_machine.system() == 'windows'
    srcs += ['fs_util_win32.cpp']
else
    srcs += ['fs_util_posix.cpp']
endif

executable(
    'vrmeditor',
    srcs,
    install: true,
    # gui_app: true,
    dependencies: [
        glew_dep,
        glfw_dep,
        imgui_dep,
        grapho_dep,
        imguizmo_dep,
        libvrm_dep,
        luajit_dep,
        cuber_dep,
        imgui_filedialog_dep,
        iconfont_cpp_headers_dep,
        imnodes_dep,
        asio_dep,
        simplefilewatcher_dep,
        stb_dep,
        imgui_colortextedit_dep,
    ],
)

install_subdir(
    'shaders',
    install_dir: 'shaders',
    install_tag: 'runtime',
    strip_directory: true,
)
install_subdir(
    'threejs_shader_chunks',
    install_dir: 'threejs_shader_chunks',
    install_tag: 'runtime',
    strip_directory: true,
)
