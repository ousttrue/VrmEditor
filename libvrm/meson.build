vrm_inc = include_directories('include')
vrm_lib = static_library(
    'vrm',
    [
        'scene.cpp',
        'glb.cpp',
        'node.cpp',
        'springbone.cpp',
    ],
    include_directories: vrm_inc,
    dependencies: [stb_dep, json_dep, geometry_dep],
)
vrm_dep = declare_dependency(
    link_with: vrm_lib,
    include_directories: vrm_inc,
    dependencies: [stb_dep, json_dep],
)
