project(
    'VrmEditor',
    ['cpp', 'c'],
    version: '1.12.0',
    license: 'MIT',
    default_options: [],
)

cdata = configuration_data()
cdata.set_quoted('PACKAGE', meson.project_name())
cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
configure_file(
    input: 'config.h.in',
    output: 'config.h',
    configuration: cdata,
)
config_dep = declare_dependency(
    include_directories: include_directories('.'),
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
imgui_filedialog_dep = subproject(
    'ImGuiFileDialog',
).get_variable('imgui_filedialog_dep')
# im_filedialog_dep = subproject(
#     'ImFileDialog',
# ).get_variable('im_filedialog_dep')
iconfont_cpp_headers_dep = dependency('iconfont_cpp_headers')
directxmath_dep = dependency('directxmath', include_type: 'system')
imnodes_dep = dependency('imnodes')
imgui_colortextedit_dep = dependency('ImGuiColorTextEdit')
asio_dep = dependency('asio')
gltfjson_dep = dependency('gltfjson')
simplefilewatcher_dep = dependency('simplefilewatcher')
# remotery_dep = subproject('remotery').get_variable('remotery_dep')
# plog_dep = subproject('sergiusthebest-plog').get_variable('plog_dep')
plog_dep = dependency('plog')
ufbx_dep = subproject('ufbx').get_variable('ufbx_dep')
ktx_dep = dependency('ktx')
subdir('recti')
subdir('boneskin')
subdir('libvrm')
subdir('glrenderer')
subdir('jsongui')

if get_option('executables')
    subdir('src')
endif
if get_option('tests')
    subdir('tests')
endif
