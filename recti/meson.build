# project('ImGuizmo', 'cpp')
# imgui_dep = dependency('imgui', default_options: ['default_library=static'])
recti_inc = include_directories('.')
recti_lib = static_library(
    'recti',
    [
        'recti.cpp',
        'imguizmo/ImGuizmo.cpp',
    ],
    include_directories: recti_inc,
    dependencies: [imgui_dep],
)
recti_dep = declare_dependency(
    link_with: recti_lib,
    include_directories: recti_inc,
)
