project(
    'ktx',
    ['c', 'cpp'],
    version: '4.2.1',
)

cdata = configuration_data()
cdata.set('PACKAGE', meson.project_name())
cdata.set('PACKAGE_VERSION', meson.project_version())
configure_file(
    input: 'version.h.in',
    output: 'version.h',
    configuration: cdata,
)
config_dep = declare_dependency(
    include_directories: include_directories('.'),
)

glew_dep = dependency('glew')
ktx_deps = [glew_dep]

ktx_args = [
    '-DBASISD_SUPPORT_FXT1=0',
    '-DBASISD_SUPPORT_KTX2=1',
    '-DBASISD_SUPPORT_KTX2_ZSTD=0',
    '-DBASISU_NO_ITERATOR_DEBUG_LEVEL',
    '-DBASISU_SUPPORT_OPENCL=0',
    '-DBASISU_SUPPORT_SSE=1',
    '-DKTX_API=__declspec(dllexport)',
    '-DKTX_FEATURE_KTX1',
    '-DKTX_FEATURE_KTX2',
    '-DKTX_FEATURE_WRITE',
    '-DLIBKTX',
    '-Dktx_EXPORTS',
]

c_compiler = meson.get_compiler('c')
transcoder_args = []
if c_compiler.get_id() == 'msvc'
    ktx_args += [
        '-DWIN32',
        '-D_WINDOWS',
    ]
    export_def = 'lib/internalexport.def'
else
    ktx_args += [
        '-Wall',
        '-Wextra',
        '-Wno-nested-anon-types',
        '-Wno-gnu-anonymous-struct',
        '-Wno-pedantic',
        '-msse4.1',
    ]
    transcoder_args += [
        '-Wno-sign-compare',
        '-Wno-unused-function',
        '-Wno-unused-variable',
        '-Wno-class-memaccess',
        '-Wno-maybe-uninitialized',
    ]
    export_def = 'lib/internalexport_mingw.def'
endif
if host_machine.system() == 'windows'
    if c_compiler.get_id() == 'msvc'
    else
        # ktx_args += [
        #     '-DWIN32_HAS_PTHREADS',
        # ]
        # pthread_dep = c_compiler.find_library('pthread', required: true)
        # ktx_deps += pthread_dep
    endif
endif

ktx_inc = include_directories(
    'include',
    'lib/basisu/transcoder',
    'lib/basisu/zstd',
    'other_include',
    'utils',
    'lib/dfdutils',
    'lib/basisu',
    'lib/astc-encoder/Source',
)

#KTX_MAIN_SRC
ktx_srcs_main = files(
    'lib/basis_transcode.cpp',
    'lib/basisu/zstd/zstd.c',
    'lib/checkheader.c',
    'lib/dfdutils/colourspaces.c',
    'lib/dfdutils/createdfd.c',
    'lib/dfdutils/interpretdfd.c',
    'lib/dfdutils/printdfd.c',
    'lib/dfdutils/queries.c',
    'lib/dfdutils/vk2dfd.c',
    'lib/etcdec.cxx',
    'lib/etcunpack.cxx',
    'lib/filestream.c',
    'lib/hashlist.c',
    'lib/info.c',
    'lib/memstream.c',
    'lib/miniz_wrapper.cpp',
    'lib/strings.c',
    'lib/swap.c',
    'lib/texture.c',
    'lib/texture2.c',
    'lib/vkformat_check.c',
    'lib/vkformat_str.c',
    # 'lib/dfdutils/vk2dfd.inl',
    # 'lib/basisu/transcoder/basisu_transcoder.cpp',
)

# BASISU_ENCODER_CXX_SRC
ktx_srcs_encoder = files(
    'lib/basisu/encoder/basisu_backend.cpp',
    'lib/basisu/encoder/basisu_basis_file.cpp',
    'lib/basisu/encoder/basisu_bc7enc.cpp',
    'lib/basisu/encoder/basisu_comp.cpp',
    'lib/basisu/encoder/basisu_enc.cpp',
    'lib/basisu/encoder/basisu_etc.cpp',
    'lib/basisu/encoder/basisu_frontend.cpp',
    'lib/basisu/encoder/basisu_gpu_texture.cpp',
    'lib/basisu/encoder/basisu_kernels_sse.cpp',
    'lib/basisu/encoder/basisu_opencl.cpp',
    'lib/basisu/encoder/basisu_pvrtc1_4.cpp',
    'lib/basisu/encoder/basisu_resample_filters.cpp',
    'lib/basisu/encoder/basisu_resampler.cpp',
    'lib/basisu/encoder/basisu_ssim.cpp',
    'lib/basisu/encoder/basisu_uastc_enc.cpp',
)

# KTX_FEATURE_GL_UPLOAD
ktx_srcs_gl = files(
    'lib/gl_funcs.c',
    'lib/glloader.c',
)

# baisisu
ktx_srcs_basisu = files(
    'lib/basisu/basisu_tool.cpp',
    'lib/basisu/encoder/basisu_backend.cpp',
    'lib/basisu/encoder/basisu_basis_file.cpp',
    'lib/basisu/encoder/basisu_bc7enc.cpp',
    'lib/basisu/encoder/basisu_comp.cpp',
    'lib/basisu/encoder/basisu_enc.cpp',
    'lib/basisu/encoder/basisu_etc.cpp',
    'lib/basisu/encoder/basisu_frontend.cpp',
    'lib/basisu/encoder/basisu_gpu_texture.cpp',
    'lib/basisu/encoder/basisu_kernels_sse.cpp',
    'lib/basisu/encoder/basisu_opencl.cpp',
    'lib/basisu/encoder/basisu_pvrtc1_4.cpp',
    'lib/basisu/encoder/basisu_resample_filters.cpp',
    'lib/basisu/encoder/basisu_resampler.cpp',
    'lib/basisu/encoder/basisu_ssim.cpp',
    'lib/basisu/encoder/basisu_uastc_enc.cpp',
    'lib/basisu/encoder/jpgd.cpp',
    'lib/basisu/encoder/pvpngreader.cpp',
    'lib/basisu/transcoder/basisu_transcoder.cpp',
)

ktx_srcs_astc = files(
    'lib/astc-encoder/Source/astcenc_averages_and_directions.cpp',
    'lib/astc-encoder/Source/astcenc_block_sizes.cpp',
    'lib/astc-encoder/Source/astcenc_color_quantize.cpp',
    'lib/astc-encoder/Source/astcenc_color_unquantize.cpp',
    'lib/astc-encoder/Source/astcenc_compress_symbolic.cpp',
    'lib/astc-encoder/Source/astcenc_compute_variance.cpp',
    'lib/astc-encoder/Source/astcenc_decompress_symbolic.cpp',
    'lib/astc-encoder/Source/astcenc_diagnostic_trace.cpp',
    'lib/astc-encoder/Source/astcenc_entry.cpp',
    'lib/astc-encoder/Source/astcenc_find_best_partitioning.cpp',
    'lib/astc-encoder/Source/astcenc_ideal_endpoints_and_weights.cpp',
    'lib/astc-encoder/Source/astcenc_image.cpp',
    'lib/astc-encoder/Source/astcenc_integer_sequence.cpp',
    'lib/astc-encoder/Source/astcenc_mathlib.cpp',
    'lib/astc-encoder/Source/astcenc_mathlib_softfloat.cpp',
    'lib/astc-encoder/Source/astcenc_partition_tables.cpp',
    'lib/astc-encoder/Source/astcenc_percentile_tables.cpp',
    'lib/astc-encoder/Source/astcenc_pick_best_endpoint_format.cpp',
    'lib/astc-encoder/Source/astcenc_quantization.cpp',
    'lib/astc-encoder/Source/astcenc_symbolic_physical.cpp',
    'lib/astc-encoder/Source/astcenc_weight_align.cpp',
    'lib/astc-encoder/Source/astcenc_weight_quant_xfer_tables.cpp',
)

transcoder_lib = static_library(
    'transcoder',
    'lib/basisu/transcoder/basisu_transcoder.cpp',
    cpp_args: transcoder_args + ktx_args,
)

ktx_lib = shared_library(
    'ktx',
    [
        'lib/texture1.c',
        'lib/basis_encode.cpp',
        'lib/astc_encode.cpp',
        'lib/writer1.c',
        'lib/writer2.c',
        ktx_srcs_main,
        ktx_srcs_encoder,
        ktx_srcs_gl,
        # ktx_srcs_basisu,
        ktx_srcs_astc,
    ],
    install: true,
    include_directories: ktx_inc,
    dependencies: ktx_deps,
    c_args: ktx_args,
    cpp_args: ktx_args,
    vs_module_defs: export_def,
    link_with: transcoder_lib,
)
ktx_dep = declare_dependency(
    include_directories: include_directories('include'),
    link_with: ktx_lib,
    dependencies: ktx_deps,
    # compile_args: ktx_args,
)
