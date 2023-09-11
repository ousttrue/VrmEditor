project('plog')
plog_inc = include_directories('include')
plog_dep = declare_dependency(
    include_directories: plog_inc,
    compile_args: [
        '-D_UTF8',
        '-DPLOG_CHAR_IS_UTF8=1',
    ],
)
