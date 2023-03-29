directxmath_dep = dependency('directxmath')
geometry_inc = include_directories('.')
dependencies = [directxmath_dep, grapho_dep]
geometry_lib = static_library(
    'gizmo',
    [
        'gizmo.cpp',
    ],
    dependencies: dependencies,
)
geometry_dep = declare_dependency(
    include_directories: geometry_inc,
    link_with: geometry_lib,
    dependencies: dependencies,
)
