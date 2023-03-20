glew_dep = dependency('glew', default_options: ['default_library=static'])
glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui')
spanmath_dep = dependency('spanmath')
json_dep = subproject('json').get_variable('nlohmann_json_dep')
glo_dep = dependency('glo')

executable(
    'vrmeditor',
    [
        'main.cpp',
        'gl3renderer.cpp',
        'gui.cpp',
        'scene.cpp',
    ],
    install: true,
    dependencies: [
        glew_dep,
        glfw_dep,
        imgui_dep,
        spanmath_dep,
        json_dep,
        glo_dep,
    ],
)
