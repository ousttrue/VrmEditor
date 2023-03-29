directxmath_dep = dependency('directxmath')
vrm_inc = include_directories('include')
dependencies = [stb_dep, json_dep, directxmath_dep, grapho_dep]
vrm_lib = static_library(
    'vrm',
    [
        'scene.cpp',
        'glb.cpp',
        'node.cpp',
        'springbone.cpp',
        'gizmo.cpp',
        'timeline.cpp',
    ],
    include_directories: vrm_inc,
    dependencies: dependencies,
)
vrm_dep = declare_dependency(
    link_with: vrm_lib,
    include_directories: vrm_inc,
    dependencies: dependencies,
)
