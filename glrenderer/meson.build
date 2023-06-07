glr_inc = include_directories('.')
glr_lib = static_library(
    'glr',
    [
        'glr/error_check.cpp',
        'glr/gl3renderer.cpp',
        'glr/shader_source.cpp',
        'glr/material_three_vrm0.cpp',
        'glr/material_three_vrm1.cpp',
        'glr/material_pbr_khronos.cpp',
    ],
    dependencies: [
        glew_dep,
        libvrm_dep,
        cuber_dep,
        plog_dep,
        remotery_dep,
        boneskin_dep,
    ],
)
glr_dep = declare_dependency(
    include_directories: glr_inc,
    link_with: glr_lib,
)

install_subdir(
    'shaders',
    install_dir: 'shaders',
    install_tag: 'runtime',
    strip_directory: true,
)
install_subdir(
    'threejs_shader_chunks',
    install_dir: 'threejs_shader_chunks',
    install_tag: 'runtime',
    strip_directory: true,
)
