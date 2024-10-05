# basing this on of https://mesonbuild.com/IndepthTutorial

project('chump', ['c', 'cpp'],
  version : '0.0.9',
  default_options : ['cpp_std=c++17', 'default_library=static'],
  license : '?')

add_global_arguments(language : ['c', 'cpp'])

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

fs = import('fs')


if build_machine.system() == 'windows'
  package_dir = fs.expanduser('~/Documents/chuck/chugins/.chump')
else
  package_dir = fs.expanduser('~/.chuck/lib/.chump')
endif

install_data('data/packages.json', install_dir : package_dir)

# Expose data for tests at local level in build dir
fs.copyfile('tests/data/test-package-list.json')
