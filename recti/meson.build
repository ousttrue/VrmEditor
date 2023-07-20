# project('ImGuizmo', 'cpp')
# imgui_dep = dependency('imgui', default_options: ['default_library=static'])
recti_inc = include_directories('.')
recti_lib = static_library(
    'recti',
    [
        'vec4.cpp',
        'mat4.cpp',
        'camera_mouse.cpp',
        'style.cpp',
        'drawcommand.cpp',
        'tripod.cpp',
        'handle/translationGizmo.cpp',
        'handle/translationDragHandle.cpp',
        'handle/rotation_gizmo.cpp',
        'handle/rotation_drag.cpp',
        'handle/scale.cpp',
        'handle/scaleDragHandle.cpp',
        'recti.cpp',
    ],
    include_directories: recti_inc,
    dependencies: [directxmath_dep],
)
recti_dep = declare_dependency(
    link_with: recti_lib,
    include_directories: recti_inc,
)
