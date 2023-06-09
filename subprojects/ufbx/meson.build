project('ufbx', 'c')
ufbx_inc = include_directories('.')
ufbx_lib = static_library('ufbx', 'ufbx.c')
ufbx_dep = declare_dependency(
    link_with: ufbx_lib,
    include_directories: ufbx_inc,
)
