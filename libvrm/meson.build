libvrm_inc = include_directories('.')
libvrm_deps = [
    directxmath_dep,
    grapho_dep,
    asio_dep,
    gltfjson_dep,
    stb_dep,
    ktx_dep,
    boneskin_dep,
]
libvrm_lib = static_library(
    'vrm',
    [
        'vrm/gltfroot.cpp',
        'vrm/node.cpp',
        'vrm/image.cpp',
        'vrm/importer.cpp',
        'vrm/runtime_scene.cpp',
        'vrm/animation.cpp',
        'vrm/timeline.cpp',
        'vrm/spring_bone.cpp',
        'vrm/spring_collision.cpp',
        'vrm/runtime_springjoint.cpp',
        'vrm/network/srht_update.cpp',
        'vrm/network/srht_sender.cpp',
        'vrm/bvh/bvh.cpp',
        'vrm/bvh/bvhframe.cpp',
        'vrm/bvh/bvhscene.cpp',
        'vrm/humanoid/humanskeleton.cpp',
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
