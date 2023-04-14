libvrm_inc = include_directories('include')
dependencies = [stb_dep, json_dep, directxmath_dep, grapho_dep]
libvrm_lib = static_library(
    'vrm',
    [
        'scene.cpp',
        'glb.cpp',
        'node.cpp',
        'springbone.cpp',
        'gizmo.cpp',
        'exporter.cpp',
        'animation.cpp',
        'bvh.cpp',
        'bvhframe.cpp',
        'bvhscene.cpp',
        'srht_update.cpp',
    ],
    include_directories: libvrm_inc,
    dependencies: dependencies,
)
libvrm_dep = declare_dependency(
    link_with: libvrm_lib,
    include_directories: libvrm_inc,
    dependencies: dependencies,
)
