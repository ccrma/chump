# Note: this entire file is llm-generated gloop
#
# Meson port of libsolv's CMake build (static libsolv + libsolvext only).
#
# Mirrors the configuration previously passed to cmake.subproject():
#   no language bindings, static only, no zstd/lzma/bzip2/zlib compression,
#   no distro-specific repo formats.
project('libsolv', 'c',
  version: '0.7.41',
  # No c_std: like upstream CMake, use the compiler default (libsolv relies on
  # C11 typedef redefinition, which warns under gnu99).
  default_options: ['warning_level=1', 'default_library=static'],
  meson_version: '>=0.56.0',
)

cc = meson.get_compiler('c')
is_windows = host_machine.system() == 'windows'

# -----------------------------------------------------------------------------
# Feature / platform definitions (CMake uses ADD_DEFINITIONS, not a config.h)
# -----------------------------------------------------------------------------
c_args = ['-DLIBSOLV_INTERNAL=1']

foreach f : ['qsort_r', '__qsort_r', 'strchrnul', 'fopencookie', 'funopen']
  if cc.has_function(f, prefix: '#define _GNU_SOURCE\n#include <stdlib.h>\n#include <string.h>\n#include <stdio.h>')
    c_args += '-DHAVE_' + f.to_upper() + '=1'
  endif
endforeach

if host_machine.endian() == 'big'
  c_args += '-DWORDS_BIGENDIAN=1'
endif

# Neither funopen nor fopencookie exist on Windows
if is_windows
  c_args += '-DWITHOUT_COOKIEOPEN=1'
endif

if cc.get_id() == 'msvc'
  c_args += ['-D_CRT_SECURE_NO_WARNINGS', '-D_CRT_NONSTDC_NO_DEPRECATE']
endif

# Static build: on Windows this switches SOLV_API away from dllimport/dllexport
public_args = []
if is_windows
  public_args += '-DSOLV_STATIC_LIB'
endif

# -----------------------------------------------------------------------------
# Generated headers
# -----------------------------------------------------------------------------
v = meson.project_version().split('.')
version_conf = configuration_data()
version_conf.set('VERSION', meson.project_version())
version_conf.set('LIBSOLV_MAJOR', v[0])
version_conf.set('LIBSOLV_MINOR', v[1])
version_conf.set('LIBSOLV_PATCH', v[2])
# All LIBSOLV_FEATURE_* / LIBSOLVEXT_FEATURE_* #cmakedefines stay undefined.

solvversion_h = configure_file(
  input: 'src/solvversion.h.in',
  output: 'solvversion.h',
  configuration: version_conf,
  format: 'cmake',
)

# -----------------------------------------------------------------------------
# Sources (from src/CMakeLists.txt and ext/CMakeLists.txt)
# -----------------------------------------------------------------------------
libsolv_src = files(
  'src/bitmap.c', 'src/poolarch.c', 'src/poolvendor.c', 'src/poolid.c',
  'src/pooldep.c', 'src/poollang.c', 'src/poolwhatprovides.c', 'src/pool.c',
  'src/strpool.c', 'src/dirpool.c', 'src/solver.c', 'src/solverdebug.c',
  'src/repo_solv.c', 'src/repo_write.c', 'src/evr.c', 'src/queue.c',
  'src/repo.c', 'src/repodata.c', 'src/repopage.c', 'src/util.c',
  'src/policy.c', 'src/solvable.c', 'src/transaction.c', 'src/order.c',
  'src/rules.c', 'src/problems.c', 'src/linkedpkg.c', 'src/cplxdeps.c',
  'src/chksum.c', 'src/solvversion.c', 'src/selection.c',
  'src/fileprovides.c', 'src/diskusage.c', 'src/suse.c', 'src/solver_util.c',
  'src/cleandeps.c', 'src/userinstalled.c', 'src/filelistfilter.c',
  'src/decision.c',
  # internal hash implementation (WITH_OPENSSL off)
  'src/chksum_impl.c', 'src/md5.c', 'src/sha1.c', 'src/sha2.c',
)

libsolvext_src = files(
  'ext/solv_xfopen.c', 'ext/testcase.c', 'ext/repo_testcase.c',
)

inc_dirs = ['src', 'ext', '.']
if is_windows
  libsolv_src += files(
    'win32/fnmatch.c', 'win32/getopt.c', 'win32/regcomp.c',
    'win32/regexec.c', 'win32/strfncs.c', 'win32/tre-mem.c',
  )
  inc_dirs += 'win32'
endif
inc = include_directories(inc_dirs)

# -----------------------------------------------------------------------------
# Targets
# -----------------------------------------------------------------------------
libsolv_lib = static_library('solv',
  libsolv_src, solvversion_h,
  include_directories: inc,
  c_args: c_args + public_args,
  pic: true,
  install: false,
)

libsolvext_lib = static_library('solvext',
  libsolvext_src, solvversion_h,
  include_directories: inc,
  c_args: c_args + public_args,
  pic: true,
  install: false,
)

libsolv_dep = declare_dependency(
  link_with: libsolv_lib,
  include_directories: inc,
  compile_args: public_args,
  sources: solvversion_h,
)

libsolvext_dep = declare_dependency(
  link_with: libsolvext_lib,
  include_directories: inc,
  dependencies: libsolv_dep,
)

meson.override_dependency('libsolv', libsolv_dep)
meson.override_dependency('libsolvext', libsolvext_dep)
