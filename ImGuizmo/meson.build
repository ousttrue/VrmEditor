imguizmo_inc = include_directories('.')
imguizmo_lib = static_library(
    'imguizmo',
    [
        'ImGuizmo.cpp',
    ],
    include_directories: imguizmo_inc,
    dependencies: [imgui_dep],
)
imguizmo_dep = declare_dependency(
    link_with: imguizmo_lib,
    include_directories: imguizmo_inc,
)
