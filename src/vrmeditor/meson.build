srcs = [
    'main.cpp',
    'platform.cpp',
    'app.cpp',
    'luahost.cpp',
    'filewatcher.cpp',
    'fbx_loader.cpp',
    'docks/asset_view.cpp',
    'docks/gui.cpp',
    'docks/dockspace.cpp',
    'docks/imlogger.cpp',
    'docks/vrm_dock.cpp',
    'docks/export_dock.cpp',
    'docks/hierarchy_gui.cpp',
    'jsongui/json_gui.cpp',
    'jsongui/jsonschema/json_prop.cpp',
    'jsongui/jsonschema/gltf.cpp',
    'jsongui/jsonschema/vrm0.cpp',
    'jsongui/jsonschema/vrm1.cpp',
    'jsongui/jsonschema/extensions.cpp',
    'jsongui/json_widgets.cpp',
    'jsongui/jsonschema/tag.cpp',
    'view/scene_preview.cpp',
    'view/im_fbo.cpp',
    'view/overlay.cpp',
    'view/lighting.cpp',
    'view/gl3renderer_gui.cpp',
    'view/animation_view.cpp',
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
    # gui_app: true,
    dependencies: [
        config_dep,
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
        ufbx_dep,
    ],
    # cpp_args: [
    #     '-DPLOG_ENABLE_WCHAR_INPUT',
    # ],
)
