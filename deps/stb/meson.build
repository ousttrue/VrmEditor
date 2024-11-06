project('stb', 'c')

stb_inc = include_directories('.')
stb_lib = static_library(
    'stb',
    [
        'stb_image.c',
    ],
    include_directories: stb_inc,
)

cc = meson.get_compiler('c')
stb_dep = declare_dependency(
    link_with: stb_lib,
    include_directories: stb_inc,
)