glr_inc = include_directories('.')
glr_lib = static_library(
    'glr',
    [
        'glr/gl3renderer.cpp',
        'glr/shader_source.cpp',
        'glr/material_three_vrm0.cpp',
        'glr/material_three_vrm1.cpp',
        'glr/material_pbr_khronos.cpp',
        'glr/gizmo.cpp',
        'glr/rendertarget.cpp',
        'glr/scene_renderer.cpp',
        'glr/material.cpp',
    ],
    dependencies: [
        glew_dep,
        libvrm_dep,
        cuber_dep,
        plog_dep,
        # remotery_dep,
        boneskin_dep,
        recti_dep,
        imgui_dep,
    ],
    cpp_args: args,
)
glr_dep = declare_dependency(
    include_directories: glr_inc,
    link_with: glr_lib,
    compile_args: args,
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
