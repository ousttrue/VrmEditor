project('ImGuiColorTextEdit', 'cpp')
imgui_dep = dependency(
    'imgui',
    default_options: ['default_library=static'],
)
imgui_colortextedit_inc = include_directories('.')
imgui_colortextedit_lib = static_library(
    'imgui_colortextedit',
    ['TextEditor.cpp'],
    include_directories: imgui_colortextedit_inc,
    dependencies: [imgui_dep],
)
imgui_colortextedit_dep = declare_dependency(
    link_with: imgui_colortextedit_lib,
    include_directories: imgui_colortextedit_inc,
)
meson.override_dependency('ImGuiColorTextEdit', imgui_colortextedit_dep)
