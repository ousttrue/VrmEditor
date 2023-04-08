gtest_main_dep = dependency('gtest_main')
executable(
    'tests',
    [
        'gltf_samples.cpp',
        'base64.cpp',
        'glb_export.cpp',
        'json.cpp',
        'vrma.cpp',
        'text.cpp',
    ],
    install: true,
    dependencies: [gtest_main_dep, vrm_dep],
    cpp_args: ['-D_CRT_SECURE_NO_WARNINGS'],
)
