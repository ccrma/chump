chump_lib_sources = [
        'chuck_version.cpp',
        'fetch.cpp',
        'manager.cpp',
        'package_list.cpp',
        'package.cpp',
        'uninstaller.cpp',
        'util.cpp',
]

# Fetch the dependency
nlohmann_json_dep = subproject('nlohmann_json').get_variable('nlohmann_json_dep')
libcurl_dep = dependency('libcurl', required: true)
openssl_dep = subproject('openssl', required: true).get_variable('openssl_dep')

# Windows needs pdcurses, linux/macos needs curses
if build_machine.system() == 'windows'
  curses_dep = pdcurses_lib_dep
else
  curses_dep = dependency('curses', static: true)
endif

# zlib = dependency('zlib', version : '1.2.11')

chump_lib = static_library(
  'chump_lib',
  sources: chump_lib_sources,
  include_directories : inc,
  dependencies : [nlohmann_json_dep, libcurl_dep, curses_dep, openssl_dep],
)

chump_lib_dep = declare_dependency(
  dependencies: [nlohmann_json_dep, libcurl_dep, curses_dep, openssl_dep],
  include_directories : inc,
  link_with : chump_lib)
