project('ImGuiFileDialog', 'cpp', version: '0.6.4')
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
stb_dep = dependency('stb')
dependencies = [imgui_dep, stb_dep]
args = ['-DIMGUI_DEFINE_MATH_OPERATORS', '-DUSE_BOOKMARK=1']
imgui_filedialog_inc = include_directories('.')
imgui_filedialog_lib = static_library(
    'ImGuiFileDialog',
    [
        'ImGuiFileDialog.cpp',
    ],
    include_directories: imgui_filedialog_inc,
    dependencies: dependencies,
    cpp_args: args,
)
imgui_filedialog_dep = declare_dependency(
    include_directories: imgui_filedialog_inc,
    link_with: imgui_filedialog_lib,
    compile_args: args,
)
