#!/bin/sh
ndk-build
rm -rf assets/lua
mkdir -p assets/lua/share
cp -r ../../share assets/lua/
