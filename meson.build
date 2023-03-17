project('VrmEditor', 'cpp', version: '0.1.0')

glew_dep = dependency('glew', default_options: ['default_library=static'])
glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui')

executable(
    'vrmeditor',
    ['src/main.cpp'],
    install: true,
    dependencies: [glew_dep, glfw_dep, imgui_dep],
)
