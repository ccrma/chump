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
# Create a build dir
mkdir build
cd build
# configure cmake
cmake . -S ../
# compile
cmake --build . -j
# run tests
ctest
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
