srcs = [
    'main.cpp',
    'platform.cpp',
    'app.cpp',
    'luahost.cpp',
    'assetdir.cpp',
    'filewatcher.cpp',
    'docks/scene_preview.cpp',
    'docks/gl3renderer_gui.cpp',
    'docks/overlay.cpp',
    'docks/gui.cpp',
    'docks/dockspace.cpp',
    'docks/imlogger.cpp',
    'docks/vrm_dock.cpp',
    'docks/export_dock.cpp',
    'docks/type_gui.cpp',
    'docks/type_gui_vrm0.cpp',
    'docks/json_gui.cpp',
    'docks/json_gui_factory.cpp',
    'docks/im_widgets.cpp',
    'docks/hierarchy_gui.cpp',
    'docks/im_fbo.cpp',
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
        boneskin_dep,
        # remotery_dep,
        plog_dep,
        glr_dep,
    ],
    # cpp_args: [
    #     '-DPLOG_ENABLE_WCHAR_INPUT',
    # ],
)
