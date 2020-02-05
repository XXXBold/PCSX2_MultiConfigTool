#!/bin/sh
#
# Run within MSYS2/mingw64 CLI to build!
#
# The executable is located within the build_* directory, after the build succeeded
#

build_dir=build_win32

meson setup $build_dir --buildtype=release --default-library=static --warnlevel=1 && ninja -C $build_dir