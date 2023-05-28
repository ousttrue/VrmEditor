project(
    'VrmEditor',
    'cpp',
    version: '0.1.0',
    default_options: [],
)
# add_global_arguments('/experimental:module', language: 'cpp')

glew_dep = dependency('glew', default_options: ['default_library=static'])
glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
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
imnodes_dep = dependency('imnodes')
asio_dep = dependency('asio')
gltfjson_dep = dependency('gltfjson')
simplefilewatcher_dep = dependency('simplefilewatcher')
subdir('libvrm')

if get_option('executables')
    subdir('src')
endif
if get_option('tests')
    subdir('tests')
endif
