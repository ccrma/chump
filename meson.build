# basing this on of https://mesonbuild.com/IndepthTutorial

project('chump', 'cpp',
  version : '0.0.9',
  default_options : ['cpp_std=c++17'],
  license : '?')

add_global_arguments(language : 'cpp')

inc = include_directories('include')

subdir('include')

# windows needs pdcurses
if build_machine.system() == 'windows'
  subdir('extern/pdcurses')
endif

subdir('src')
subdir('chump-cli')
subdir('tests')

# Expose data for tests at local level in build dir
fs = import('fs')
fs.copyfile('tests/data/test-package-list.json')
