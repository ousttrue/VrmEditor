project(
    'VrmEditor',
    ['cpp', 'c'],
    version: '1.14.8',
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
# luajit_dep = subproject(
#     'luajit',
#     default_options: ['default_library=static'],
# ).get_variable('lua_dep')
lua_dep = subproject(
    'lua',
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
directxmath_dep = dependency('directxmath')
imnodes_dep = dependency('imnodes')
imgui_colortextedit_dep = dependency('ImGuiColorTextEdit')
asio_dep = dependency('asio')
gltfjson_dep = dependency('gltfjson')
simplefilewatcher_dep = dependency('simplefilewatcher')
# remotery_dep = subproject('remotery').get_variable('remotery_dep')
# plog_dep = subproject('sergiusthebest-plog').get_variable('plog_dep')
plog_dep = dependency('plog')
ufbx_dep = subproject('ufbx').get_variable('ufbx_dep')

ktx_dep = dependency('ktx', default_options: ['cpp_std=c++17'])
# cmake = import('cmake')
# opt_var = cmake.subproject_options()
# opt_var.set_override_option('cpp_std', 'c++17')
# opt_var.add_cmake_defines(
#     {
#         'KTX_FEATURE_TOOLS': false,
#         'KTX_FEATURE_TESTS': false,
#     },
# )
# ktx_cmake = cmake.subproject('ktx', options: opt_var)
# ktx_dep = ktx_cmake.dependency('ktx')

args = []
cpp = meson.get_compiler('cpp')
if cpp.get_id() == 'clang'
    args += '-Wno-defaulted-function-deleted'
endif

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
