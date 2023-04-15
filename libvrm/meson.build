libvrm_inc = include_directories('include')
libvrm_deps = [
    stb_dep,
    json_dep,
    directxmath_dep,
    grapho_dep,
    asio_dep,
]
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
        'timeline.cpp',
        'srht_update.cpp',
        'srht_sender.cpp',
    ],
    include_directories: libvrm_inc,
    dependencies: libvrm_deps,
    cpp_args: ['-DNOMINMAX'],
)
libvrm_dep = declare_dependency(
    link_with: libvrm_lib,
    include_directories: libvrm_inc,
    dependencies: libvrm_deps,
    compile_args: ['-DNOMINMAX'],
)
