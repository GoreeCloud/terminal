project(
  'goreecloud-terminal',
  'c',
  version: '0.1.0-dev',
  meson_version: '>=1.0.0',
  default_options: ['c_std=c17', 'warning_level=3'],
)

gtk_dep = dependency('gtk4', version: '>=4.12')
vte_dep = dependency('vte-2.91-gtk4', version: '>=0.76')
glib_dep = dependency('glib-2.0', version: '>=2.76')
gio_dep = dependency('gio-2.0', version: '>=2.76')
pcre2_dep = dependency('libpcre2-8')

subdir('src')
subdir('tests')

install_data(
  'data/com.goreecloud.Terminal.desktop',
  install_dir: join_paths(get_option('datadir'), 'applications'),
)
