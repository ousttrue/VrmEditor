runtimescene_inc = include_directories('.')
runtimescene_lib = static_library(
    'runtimescene',
    [
        'animation_update.cpp',
        'springjoint.cpp',
        'scene_updater.cpp',
    ],
    include_directories: runtimescene_inc,
    dependencies: [libvrm_dep],
)
runtimescene_dep = declare_dependency(
    link_with: runtimescene_lib,
    include_directories: runtimescene_inc,
)
