project('im_neo_sequencer', 'cpp')
imgui_dep = dependency('imgui')
im_neo_sequencer_inc = include_directories('.')
im_neo_sequencer_lib = static_library(
    'im_neo_sequencer',
    [
        'imgui_neo_internal.cpp',
        'imgui_neo_sequencer.cpp',
    ],
    include_directories: im_neo_sequencer_inc,
    dependencies: [imgui_dep],
)
im_neo_sequencer_dep = declare_dependency(
    link_with: im_neo_sequencer_lib,
    include_directories: im_neo_sequencer_inc,
)
