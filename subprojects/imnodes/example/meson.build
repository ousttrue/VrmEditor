glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
gl_dep = dependency('GL')
executable(
    'color_node_editor',
    [
        'color_node_editor.cpp',
        'main.cpp',
    ],
    install: true,
    dependencies: [imnodes_dep, glfw_dep, gl_dep],
)
