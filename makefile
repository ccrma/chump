
########################## DEFAULT MAKE TARGET ################################
# default target: print usage message and quit
current:
	@echo "[chump build]: please use one of the following configurations:"
	@echo "   make mac, make linux, make win"

# creates build-release
build-release:
	meson setup builddir-release

build-debug:
	meson setup --buildtype=debug --debug builddir-debug -Db_coverage=true

build-release-win:
	meson setup builddir-release --backend vs

build-release-mac:
	meson setup builddir-release --backend xcode

setup-mac-x86_64:
	meson setup --cross-file cross/x86_64-macos.txt builddir-x86_64

setup-mac-arm64:
	meson setup --cross-file cross/arm64-macos.txt builddir-arm64

build-mac-x86_64: setup-mac-x86_64
	meson compile -C builddir-x86_64

build-mac-arm64: setup-mac-arm64
	meson compile -C builddir-arm64

install:
	meson install -C builddir-release

coverage: build-debug
	ninja coverage -C builddir-debug

.PHONY: linux linux-oss linux-jack linux-alsa linux-all
linux linux-oss linux-jack linux-alsa linux-all: build-release
	meson compile -C builddir-release

.PHONY: mac osx
mac osx: mac-universal

linux-debug: build-debug
	meson compile -C builddir-debug

mac-universal: build-mac-x86_64 build-mac-arm64
	lipo -create -output chump builddir-x86-64/chump-cli/chump builddir-arm64/chump-cli/chump
	lipo -create -output Chumpinate.chug builddir-x86-64/chumpinate/Chumpinate.chug builddir-arm64/chumpinate/Chumpinate.chug

.PHONY: win win32 win64
win win32 win64: build-release-win
	meson compile -C builddir-release

test:
	meson test -C builddir-release -v chump:

chumpinate_pkg_linux: linux chumpinate_ckdoc
	cd chumpinate; chuck -s build-pkg-linux.ck

chumpinate_pkg_win: win chumpinate_ckdoc
	cd chumpinate; chuck -s build-pkg-win.ck

chumpinate_pkg_mac: mac chumpinate_ckdoc
	cd chumpinate; chuck -s build-pkg-mac.ck

chumpinate_ckdoc:
	cd chumpinate; chuck -s --chugin-path:../builddir-release/chumpinate gen-ckdoc.ck

clean:
ifneq ("$(wildcard builddir-release)","")
	meson compile -C builddir-release --clean
endif
ifneq ("$(wildcard builddir-debug)","")
	meson compile -C builddir-debug --clean
endif

clean-all:
	rm -rf builddir-release builddir-debug builddir-x86_64 builddir-arm64
