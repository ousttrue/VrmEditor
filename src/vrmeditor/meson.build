srcs = [
    'main.cpp',
    'platform.cpp',
    'app.cpp',
    'luahost.cpp',
    'assetdir.cpp',
    'glr/gl3renderer.cpp',
    'glr/cuber.cpp',
    'glr/scene_preview.cpp',
    'glr/rendertarget.cpp',
    'glr/line_gizmo.cpp',
    'glr/overlay.cpp',
    'docks/gui.cpp',
    'docks/scene_dock.cpp',
    'docks/scene_gui.cpp',
    'docks/vrm_dock.cpp',
    'docks/export_dock.cpp',
    'docks/json_dock.cpp',
    'docks/json_gui.cpp',
    'docks/json_gui_accessor.cpp',
    'docks/json_gui_material.cpp',
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
    gui_app: true,
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
    ],
)
