
########################## DEFAULT MAKE TARGET ################################
# default target: print usage message and quit
current:
	@echo "[chump build]: please use one of the following configurations:"
	@echo "   make mac, make linux, make win"

# creates build-release
build-release:
	meson setup builddir-release

build-release-win:
	meson setup builddir-release --backend vs

install:
	meson install -C builddir-release

.PHONY: mac osx linux linux-oss linux-jack linux-alsa linux-all
mac osx linux linux-oss linux-jack linux-alsa linux-all: build-release
	meson compile -C builddir-release

.PHONY: win win32 win64
win win32 win64: build-release-win
	meson compile -C builddir-release

# (mac/linux) remove system installed ChuGL and install local ChuGL
test:
	meson test -C builddir-release -v chump:

clean:
ifneq ("$(wildcard builddir-release)","")
	meson compile -C builddir-release --clean
endif
ifneq ("$(wildcard builddir-debug)","")
	meson compile -C builddir-debug --clean
endif

clean-all: 
	rm -rf builddir-release builddir-debug
