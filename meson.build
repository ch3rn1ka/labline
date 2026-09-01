project(
    'labline',
    'c',
    version: '0.20.1',
    license: 'GPL-2.0-only',
    meson_version: '>=1.8.0',
    default_options: [
        'c_std=c11',
        'warning_level=1'
    ],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
  '-pedantic',
  '-D_POSIX_C_SOURCE=200809L',
]), language: 'c')

wayland_client = dependency('wayland-client', version: '>=1.22.90')
wayland_protos = dependency('wayland-protocols', version: '>=1.39')
cairo          = dependency('cairo')
pangocairo     = dependency('pangocairo')

labline_inc = include_directories('include')

subdir('protocols')
subdir('src')

labline_deps = [
    wayland_client,
    cairo,
    pangocairo,
    protocol_dep
]

executable(
    meson.project_name(),
    labline_sources,
    include_directories: [labline_inc],
    dependencies: labline_deps,
    install: true
)
