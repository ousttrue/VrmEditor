directxmath_dep = dependency('directxmath')

bvhutil_lib = static_library(
    'bvhutil',
    [
        'Bvh.cpp',
        'BvhSolver.cpp',
        'BvhNode.cpp',
        'BvhFrame.cpp',
    ],
    dependencies: [
        imgui_dep,
        directxmath_dep,
    ],
)
bvhutil_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: bvhutil_lib,
)
