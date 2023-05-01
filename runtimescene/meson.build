runtimescene_inc = include_directories('.')
runtimescene_lib = static_library(
    'runtimescene',
    [
        'runtime_node.cpp',
    ],
    include_directories: runtimescene_inc,
)
runtimescene_dep = declare_dependency(
    include_directories: runtimescene_inc,
)
