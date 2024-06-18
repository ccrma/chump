![CI/CD Badge](https://github.com/ccrma/chump/actions/workflows/build-unit-tests.yml/badge.svg)

# chump
## [As in sucker](https://www.merriam-webster.com/thesaurus/chump)

The ChucK Manager of Packages

## Dependencies

### Linux
`apt-get install libssl-dev libncurses5-dev libncursesw5-dev`



## Building & Testing

### Linux
```
# configure build dir
meson setup builddir
# Go to build dir
cd builddir
# Compile the project
meson compile
# Run unit tests
meson test
```

### Windows
```
# configure build dir
meson setup builddir --backend vs
# Go to build dir
cd builddir
# Compile the project
meson compile
# Run unit tests
meson test
```
