![CI/CD Badge](https://github.com/ccrma/chump/actions/workflows/build-unit-tests.yml/badge.svg)

# chump
## [As in sucker](https://www.merriam-webster.com/thesaurus/chump)

The ChucK Manager of Packages

## Dependencies
`apt-get install libssl-dev libncurses5-dev libncursesw5-dev`

## Building & Testing


```
# create build directory in build/
cmake -Bbuild/
# compile
cmake --build build/ -j
# run tests
cd build
./tests
```
