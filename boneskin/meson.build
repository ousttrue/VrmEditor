boneskin_inc = include_directories('.')
boneskin_lib = static_library(
    'boneskin',
    [
        'boneskin/skinning_manager.cpp',
    ],
    dependencies: [
        gltfjson_dep,
        libvrm_dep,
    ],
)
boneskin_dep = declare_dependency(
    include_directories: boneskin_inc,
    link_with: boneskin_lib,
)
