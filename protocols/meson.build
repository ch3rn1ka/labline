wl_mod = import('wayland')
project_root = meson.project_source_root()

ext_workspace_xml = wl_mod.find_protocol(
    'ext-workspace',
    state: 'staging',
    version: 1
)
ext_workspace = wl_mod.scan_xml(ext_workspace_xml)

xdg_shell_xml = wl_mod.find_protocol('xdg-shell')
xdg_shell = wl_mod.scan_xml(xdg_shell_xml)

layer_shell_xml = project_root / 'protocols/wlr-layer-shell-unstable-v1.xml'
layer_shell = wl_mod.scan_xml(layer_shell_xml)

toplevel_manager_xml = project_root / 'protocols/wlr-foreign-toplevel-management-unstable-v1.xml'
toplevel_manager = wl_mod.scan_xml(toplevel_manager_xml)

protocol_srcs = []
protocol_srcs += ext_workspace
protocol_srcs += xdg_shell
protocol_srcs += layer_shell
protocol_srcs += toplevel_manager

protocol_dep = declare_dependency(
    sources: protocol_srcs
)
