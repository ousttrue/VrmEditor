project(
    'VrmEditor',
    'cpp',
    version: '0.1.0',
    default_options: ['cpp_std=c++latest'],
)

glew_dep = dependency('glew', default_options: ['default_library=static'])
glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui')
json_dep = subproject('json').get_variable('nlohmann_json_dep')
glo_dep = dependency('glo')
stb_dep = dependency('stb')
im_neo_sequencer_dep = dependency('im_neo_sequencer')

subdir('ImGuizmo')
subdir('src')
