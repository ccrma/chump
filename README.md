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
# create build directory in build/
cmake -Bbuild/
# compile
cmake --build build/ -j
# run tests
cmake --build build --target test
```

### Windows
```
# create build directory in build/
cmake -Bbuild
# compile
cmake --build build -j
# run tests
cd build
# configure your build dir
ctest -C Debug # or Release
ctest
```