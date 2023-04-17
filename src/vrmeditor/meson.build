srcs = [
    'main.cpp',
    'platform.cpp',
    'app.cpp',
    'luahost.cpp',
    'assetdir.cpp',
    'docks/gl3renderer.cpp',
    'docks/gui.cpp',
    'docks/json_dock.cpp',
    'docks/scene_dock.cpp',
    'docks/vrm_dock.cpp',
    'humanpose/motion_dock.cpp',
    'humanpose/humanpose_stream.cpp',
    'humanpose/cuber.cpp',
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
