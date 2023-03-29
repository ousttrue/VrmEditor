project(
    'VrmEditor',
    'cpp',
    version: '0.1.0',
    default_options: [],
)

glew_dep = dependency('glew', default_options: ['default_library=static'])
glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui')
json_dep = subproject('json').get_variable('nlohmann_json_dep')
grapho_dep = dependency('grapho')
stb_dep = dependency('stb')
im_neo_sequencer_dep = dependency('im_neo_sequencer')
luajit_dep = subproject(
    'luajit',
    default_options: ['default_library=static'],
).get_variable('lua_dep')
cuber_dep = dependency('cuber')

subdir('geometry')
subdir('ImGuizmo')
subdir('libvrm')
subdir('bvhutil')
subdir('src')
