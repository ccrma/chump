chump_lib_sources = [
        'chuck_version.cpp',
        'fetch.cpp',
        'manager.cpp',
        'package_list.cpp',
        'package.cpp',
        'util.cpp',
	'exec.cpp'
]

# Fetch the dependency
nlohmann_json_dep = subproject('nlohmann_json').get_variable('nlohmann_json_dep')
openssl_dep = subproject('openssl', default_options: ['prefer_static=true'], required: true).get_variable('openssl_dep')
zlib_dep = subproject('zlib', default_options: ['prefer_static=true'], required: true).get_variable('zlib_dep')

libcurl_dep = subproject('curl',
    default_options: [
        'prefer_static=true',
        'http2=disabled',
        'idn=disabled',
        # Don't let curl pick up optional system/homebrew libraries that
        # would end up dynamically linked into the executable.
        'zstd=disabled',
        'brotli=disabled',
        'ssh=disabled',
        'gss-api=disabled',
        'gsasl=disabled',
        'psl=disabled',
        'ldap=disabled',
        'ldaps=disabled',
        'rtmp=disabled',
    ],
    required: true
).get_variable('curl_dep')

minizip_dep = subproject('minizip-ng',
    default_options: [
        'prefer_static=true',
        'zstd=disabled',
        'lzma=disabled',
        'bzip2=disabled',
        'iconv=disabled',
    ],
    required: true
).get_variable('minizip_ng_dep')

libsolv_subproj = subproject('libsolv')

libsolv_dep = libsolv_subproj.get_variable('libsolv_dep')
libsolvext_dep = libsolv_subproj.get_variable('libsolvext_dep')

cheaders_proj = subproject('cheaders')
cheaders_dep = cheaders_proj.get_variable('cheaders_dep')

chump_lib = static_library(
  'chump_lib',
  sources: chump_lib_sources,
  include_directories : inc,
  dependencies : [nlohmann_json_dep, libcurl_dep, openssl_dep, minizip_dep, cheaders_dep, zlib_dep, libsolv_dep],
)

chump_lib_dep = declare_dependency(
  dependencies: [nlohmann_json_dep, libcurl_dep, openssl_dep, minizip_dep, cheaders_dep, zlib_dep, libsolv_dep],
  include_directories : inc,
  link_with : chump_lib)
