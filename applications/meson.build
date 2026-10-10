# Shell Settings' entry beside the build, for a DEsk run from the build
# directory: it looks in applications/ next to its own binary last (apps.c).
configure_file(
  input: '../session/desk-settings.desktop.in',
  output: 'desk-settings.desktop',
  configuration: { 'EXEC': meson.project_build_root() / 'desk-settings',
    'ICON': meson.project_source_root() / 'session/desk-settings.png' },
)
configure_file(
  input: '../session/desk-index.desktop.in',
  output: 'desk-index.desktop',
  configuration: { 'EXEC': meson.project_build_root() / 'desk-index',
    'ICON': meson.project_source_root() / 'session/desk-settings.png' },
)
