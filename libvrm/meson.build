libvrm_inc = include_directories('.')
libvrm_deps = [
    directxmath_dep,
    grapho_dep,
    asio_dep,
    gltfjson_dep,
]
libvrm_lib = static_library(
    'vrm',
    [
        'vrm/jsonpath.cpp',
        'vrm/scene.cpp',
        'vrm/node.cpp',
        'vrm/springbone.cpp',
        'vrm/exporter.cpp',
        'vrm/bvh.cpp',
        'vrm/bvhframe.cpp',
        'vrm/bvhscene.cpp',
        'vrm/timeline.cpp',
        'vrm/srht_update.cpp',
        'vrm/srht_sender.cpp',
        'vrm/importer.cpp',
        'vrm/springcollider.cpp',
        'vrm/runtimescene/scene.cpp',
        'vrm/runtimescene/animation_update.cpp',
        'vrm/runtimescene/springjoint.cpp',
        'vrm/runtimescene/springcollision.cpp',
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
