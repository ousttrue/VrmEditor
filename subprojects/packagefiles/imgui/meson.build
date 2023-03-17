project('imgui', 'cpp')

glfw_dep = dependency('glfw3', default_options: ['default_library=static'])

imgui_inc = include_directories('.', 'backends')

imgui_lib = static_library(
    'imgui',
    [
        'imgui.cpp',
        'imgui_demo.cpp',
        'imgui_draw.cpp',
        'imgui_tables.cpp',
        'imgui_widgets.cpp',
        'backends/imgui_impl_glfw.cpp',
        'backends/imgui_impl_opengl3.cpp',
    ],
    dependencies: glfw_dep,
)

imgui_dep = declare_dependency(
    link_with: imgui_lib,
    include_directories: imgui_inc,
)
