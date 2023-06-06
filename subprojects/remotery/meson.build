project('remotery', 'c')
remotery_inc = include_directories('lib')
remotery_lib = static_library(
    'remotery',
    [
        'lib/remotery.c',
    ],
    include_directories: remotery_inc,
)
remotery_dep = declare_dependency(
    link_with: remotery_lib,
    include_directories: remotery_inc,
)
