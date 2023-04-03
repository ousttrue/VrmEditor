project(
    'VrmEditor',
    'cpp',
    version: '0.1.0',
    default_options: [],
)

glew_dep = dependency('glew', default_options: ['default_library=static'])
glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
json_dep = dependency('nlohmann_json', include_type: 'system')
grapho_dep = dependency('grapho')
stb_dep = dependency('stb')
# im_neo_sequencer_dep = dependency('im_neo_sequencer')
luajit_dep = subproject(
    'luajit',
    default_options: ['default_library=static'],
).get_variable('lua_dep')
cuber_dep = dependency('cuber')
imguizmo_dep = subproject('ImGuizmo').get_variable('imguizmo_dep')
imgui_filedialog_dep = subproject(
    'ImGuiFileDialog',
).get_variable('imgui_filedialog_dep')
# im_filedialog_dep = subproject(
#     'ImFileDialog',
# ).get_variable('im_filedialog_dep')
iconfont_cpp_headers_dep = dependency('iconfont_cpp_headers')
directxmath_dep = dependency('directxmath', include_type: 'system')
subdir('libvrm')
subdir('bvhutil')
subdir('src')
subdir('tests')
