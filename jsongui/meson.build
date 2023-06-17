jsongui_inc = include_directories('.')
jsongui_lib = static_library(
    'jsongui',
    [
        'json_gui.cpp',
        'jsonschema/json_prop.cpp',
        'jsonschema/gltf.cpp',
        'jsonschema/vrm0.cpp',
        'jsonschema/vrm1.cpp',
        'jsonschema/extensions.cpp',
        'json_widgets.cpp',
        'jsonschema/tag.cpp',
    ],
    include_directories: jsongui_inc,
    dependencies: [
        gltfjson_dep,
        libvrm_dep,
        imgui_dep,
        glr_dep,
        plog_dep,
    ],
)
jsongui_dep = declare_dependency(
    include_directories: jsongui_inc,
    link_with: jsongui_lib,
)
