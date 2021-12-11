#!/bin/sh
#
# The executable is located within the build_* directory, after the build succeeded
#

build_dir=build_linux

meson setup $build_dir --buildtype=release --warnlevel=1 && ninja -C $build_dir