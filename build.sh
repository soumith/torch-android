#!/bin/bash
####################################################
# You do not need to modify anything below this line
####################################################
# find system torch, if not found, install it
command -v torch-lua >/dev/null 2>&1
TORCHINSTALLCHECK=$?
if [ $TORCHINSTALLCHECK -ne 0 ]; then
    echo "Torch-7 not found on system. Installing."
    curl -s https://raw.github.com/torch/ezinstall/master/install-all | bash
fi
# have ndk-build in your PATH and the script figures out where your ANDROID_NDK is at
unamestr=`uname`
ndkbuildloc=`which ndk-build`
if [[ "$unamestr" == 'Linux' ]]; then
    export ANDROID_NDK=`readlink -f $ndkbuildloc|sed 's/ndk-exec.sh//'`
elif [[ "$unamestr" == 'Darwin' ]]; then
    brew install coreutils
    export ANDROID_NDK=`greadlink -f $ndkbuildloc|sed 's/ndk-exec.sh//'`
fi
echo "Android NDK found at: $ANDROID_NDK"
cd "$(dirname "$0")" # switch to script directory
INSTALL_DIR=`pwd`
cd src
rm -f build/CMakeCache.txt
mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake .. -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DANDROID_STL=none
CMAKERET=$?
if [ $CMAKERET -ne 0 ]; then
 exit $CMAKERET
fi
make install
MAKERET=$?
if [ $MAKERET -ne 0 ]; then
 exit $MAKERET
fi
cd ../../

# copy libs
rm -rf lib
mkdir -p lib
cp src/libs/armeabi-v7a/*.a lib/

# export lua sources
rm -rf share
mkdir -p share/lua/5.1/torch
cp -r src/pkg/torch/*.lua share/lua/5.1/torch/

mkdir -p share/lua/5.1/dok
cp -r src/pkg/dok/*.lua share/lua/5.1/dok/

mkdir -p share/lua/5.1/nn
cp -r src/3rdparty/nn/*.lua share/lua/5.1/nn/

mkdir -p share/lua/5.1/image
cp -r src/3rdparty/image/*.lua share/lua/5.1/image/

mkdir -p share/lua/5.1/nnx
cp -r src/3rdparty/nnx/*.lua share/lua/5.1/nnx/

mkdir -p share/lua/5.1/imgraph
cp -r src/3rdparty/imgraph/*.lua share/lua/5.1/imgraph/

#remove cmake files in framework
rm -rf share/cmake
