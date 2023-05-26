gtest_main_dep = dependency('gtest_main')
executable(
    'tests',
    [
        'text.cpp',
    ],
    install: true,
    dependencies: [
        gtest_main_dep,
        libvrm_dep,
        gltfjson_dep,
        json_dep,
    ],
    cpp_args: ['-D_CRT_SECURE_NO_WARNINGS'],
)
