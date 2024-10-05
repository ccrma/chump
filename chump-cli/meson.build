cli11_dep = dependency('cli11', required: true) # cli args lib dep

executable(
  'chump', 'main.cpp',
  include_directories : inc,
  install: true,
  dependencies: [chump_lib_dep, cli11_dep]
)
