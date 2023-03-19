project(
    'VrmEditor',
    'cpp',
    version: '0.1.0',
    default_options: ['cpp_std=c++20'],
)

glew_dep = dependency('glew', default_options: ['default_library=static'])
glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui')
spanmath_dep = dependency('spanmath')

executable(
    'vrmeditor',
    [
        'src/main.cpp',
        'src/gl3renderer.cpp',
        'src/gui.cpp',
    ],
    install: true,
    dependencies: [
        glew_dep,
        glfw_dep,
        imgui_dep,
        spanmath_dep,
    ],
)
