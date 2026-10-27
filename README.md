![CI/CD Badge](https://github.com/ccrma/chump/actions/workflows/build-unit-tests.yml/badge.svg)

# chump

![chump_logo](https://github.com/user-attachments/assets/6e0b9c20-de6b-4854-9e41-2285f1c24ca1)

## [What is ChuMP?](https://www.merriam-webster.com/thesaurus/chump)

The ChucK Manager of Packages, ChucK's official package manager. For more information, see [https://chuck.stanford.edu/chump/](https://chuck.stanford.edu/chump/), for *even more* information, feel free to read our paper: [ChuMP and the Zen of Package Management](https://ccrma.stanford.edu/groups/mcd/publish/files/2025-nime-chump.pdf) (presented at New Interfaces for Musical Expression 2025).

## Building & Testing

### Linux
```
make linux # build
make test # run unit and integration tests
make install # install chump (can require sudo)
```

### Macs
```
make mac # build
make test # run unit and integration tests
make install # install chump (can require sudo)
```

To build a universal binary:
```
make mac-universal
```

### Windows
```
# configure build dir
meson setup --buildtype=release builddir-release --backend vs
# Go to build dir
cd builddir
# Compile the project
meson compile
# Run unit tests
meson test

# If you want to run chump.exe, you need to run it in a meson devenv:
meson devenv -C .\builddir\
.\chump-cli\chump.exe

```
