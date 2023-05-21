libvrm_inc = include_directories('.')
libvrm_deps = [
    directxmath_dep,
    grapho_dep,
    asio_dep,
    gltfjson_dep,
    stb_dep,
]
libvrm_lib = static_library(
    'vrm',
    [
        'vrm/gltfroot.cpp',
        'vrm/node.cpp',
        'vrm/image.cpp',
        'vrm/exporter.cpp',
        'vrm/importer.cpp',
        'vrm/animation/runtime_scene.cpp',
        'vrm/animation/animation_update.cpp',
        'vrm/animation/timeline.cpp',
        'vrm/animation/spring_bone.cpp',
        'vrm/animation/spring_collision.cpp',
        'vrm/animation/runtime_springjoint.cpp',
        'vrm/network/srht_update.cpp',
        'vrm/network/srht_sender.cpp',
        'vrm/bvh/bvh.cpp',
        'vrm/bvh/bvhframe.cpp',
        'vrm/bvh/bvhscene.cpp',
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
