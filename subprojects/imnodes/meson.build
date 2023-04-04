project('imnodes', 'cpp', version: 'v0.5.0')
imgui_dep = dependency(
    'imgui',
    default_options: ['default_library=static'],
)
imnodes_inc = include_directories('.')
imnodes_lib = static_library(
    'imnodes',
    'imnodes.cpp',
    include_directories: imnodes_inc,
    dependencies: [imgui_dep],
    cpp_args: ['-DIMGUI_DEFINE_MATH_OPERATORS=1'],
)
imnodes_dep = declare_dependency(
    include_directories: imnodes_inc,
    link_with: imnodes_lib,
    dependencies: [imgui_dep],
)
meson.override_dependency('imnodes', imnodes_dep)

subdir('example')
