project('stb', 'cpp')
stb_inc = include_directories('.')
stb_lib = static_library(
    'stb',
    'stb_image.cpp',
    include_directories: stb_inc,
)
stb_dep = declare_dependency(
    link_with: stb_lib,
    include_directories: stb_inc,
)
