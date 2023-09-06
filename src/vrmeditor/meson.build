srcs = [
    'main.cpp',
    'platform.cpp',
    'app.cpp',
    'luahost.cpp',
    'filewatcher.cpp',
    'fbx_loader.cpp',
    'scene_state.cpp',
    'docks/asset_view.cpp',
    'docks/gui.cpp',
    'docks/dockspace.cpp',
    'docks/imlogger.cpp',
    'docks/vrm_gui.cpp',
    'docks/export_dock.cpp',
    'docks/hierarchy_gui.cpp',
    'docks/humanoid_dock.cpp',
    'docks/springbone_gui.cpp',
    'view/scene_preview.cpp',
    'view/im_fbo.cpp',
    'view/overlay.cpp',
    'view/lighting.cpp',
    'view/gl3renderer_gui.cpp',
    'view/animation_view.cpp',
    'view/mesh_gui.cpp',
    'humanpose/humanpose_stream.cpp',
    'humanpose/udp_receiver.cpp',
    'humanpose/bvhnode.cpp',
    'humanpose/udpnode.cpp',
]

if host_machine.system() == 'windows'
    srcs += ['fs_util_win32.cpp']
    windows = import('windows')
    srcs += windows.compile_resources('icon.rc')
else
    srcs += ['fs_util_posix.cpp']
endif

executable(
    'vrmeditor',
    srcs,
    install: true,
    gui_app: get_option('buildtype') == 'release',
    dependencies: [
        config_dep,
        glfw_dep,
        imgui_dep,
        grapho_dep,
        recti_dep,
        libvrm_dep,
        lua_dep,
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
        ufbx_dep,
        jsongui_dep,
    ],
    # cpp_args: [
    #     '-DPLOG_ENABLE_WCHAR_INPUT',
    # ],
)
