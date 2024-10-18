# basing this on of https://mesonbuild.com/IndepthTutorial

project('chump', ['c', 'cpp'],
  version : '0.0.9',
  default_options : ['cpp_std=c++17', 'default_library=static'],
  license : '?')

add_global_arguments(language : ['c', 'cpp'])

if host_machine.system() == 'darwin'
  # Set minimum macOS version only for macOS
  add_project_arguments('-mmacosx-version-min=12', language : 'c')
  add_project_arguments('-mmacosx-version-min=12', language : 'cpp')
endif

inc = [include_directories('include')]

subdir('include')

# windows needs pdcurses
if build_machine.system() == 'windows'
  subdir('extern/pdcurses')
  inc += include_directories('extern/pdcurses')
endif

subdir('src')
subdir('chump-cli')
subdir('tests')
subdir('scripts')

fs = import('fs')


if build_machine.system() == 'windows'
  package_dir = fs.expanduser('~/Documents/chuck/chugins/.chump')
else
  package_dir = fs.expanduser('~/.chuck/lib/.chump')
endif

install_data('data/manifest.json',
             install_dir : package_dir,
             install_mode: 'rwxrwxrwx',
            )

# Expose data for tests at local level in build dir
fs.copyfile('tests/data/test-package-list.json')
