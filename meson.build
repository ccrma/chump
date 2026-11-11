# basing this on of https://mesonbuild.com/IndepthTutorial

project('chump', ['c', 'cpp'],
  version : '0.0.4',
  default_options : ['cpp_std=c++20', 'default_library=static'],
  license : '?')

add_global_arguments(language : ['c', 'cpp'])

if host_machine.system() == 'darwin'
  # Set minimum macOS version only for macOS
  add_project_arguments('-mmacosx-version-min=12', language : 'c')
  add_project_arguments('-mmacosx-version-min=12', language : 'cpp')

# https://github.com/openSUSE/libsolv/issues/372
  # requires is a keyword in c++20, this flag prepends dep_
  # to conflicting keywords in libsolv
  add_project_arguments('-DLIBSOLV_SOLVABLE_PREPEND_DEP', language: 'cpp')
endif


inc = [include_directories('include')]

subdir('include')

subdir('src')
subdir('chump-cli')
subdir('tests')
subdir('scripts')

fs = import('fs')


if build_machine.system() == 'windows'
  package_dir = fs.expanduser('~/Documents/ChucK/packages')
else
  package_dir = fs.expanduser('~/.chuck/packages')
endif

install_data('data/manifest.json',
             install_dir : package_dir,
             install_mode: 'rwxrwxrwx',
            )


# Expose data for tests at local level in build dir
message('copying test data...')
fs.copyfile('tests/data/test-package-list.json')

# Chumpinate options
subdir('chumpinate')

