project('simplefilewatcher', 'cpp')

simplefilewatcher_inc = include_directories('trunk/include')
simplefilewatcher_lib = static_library(
    'simplefilewatcher',
    [
        'trunk/source/FileWatcher.cpp',
        'trunk/source/FileWatcherWin32.cpp',
    ],
    include_directories: simplefilewatcher_inc,
)

simplefilewatcher_dep = declare_dependency(
    link_with: simplefilewatcher_lib,
    include_directories: simplefilewatcher_inc,
)
