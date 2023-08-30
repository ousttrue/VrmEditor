boneskin_inc = include_directories('.')
boneskin_lib = static_library(
    'boneskin',
    [
        'boneskin/meshdeformer.cpp',
        'boneskin/deformed_mesh.cpp',
    ],
    dependencies: [
        gltfjson_dep,
        directxmath_dep,
    ],
)
boneskin_dep = declare_dependency(
    include_directories: boneskin_inc,
    link_with: boneskin_lib,
)
