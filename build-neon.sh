#!/bin/bash
# have ndk-build in your $PATH and the script figures out where your ANDROID_NDK is at
####################################################
# You do not need to modify anything below this line
####################################################
# find system torch, if not found, install it
command -v th >/dev/null 2>&1
TORCHINSTALLCHECK=$?
if [ $TORCHINSTALLCHECK -ne 0 ]; then
    echo "Torch-7 not found on system. Please install it using instructions from http://torch.ch"
    exit -1
fi
# have ndk-build in your PATH and the script figures out where your ANDROID_NDK is at
unamestr=`uname`
ndkbuildloc=`which ndk-build`
if [[ "$?" == 1 ]]; then
    echo "Error: Cannot find ndk-build in PATH. Please add it to PATH environment variable"
    exit 1
fi
if [[ "$unamestr" == 'Linux' ]]; then
    export ANDROID_NDK=`readlink -f $ndkbuildloc|sed 's/ndk-exec.sh//'|sed 's/ndk-build//'`
elif [[ "$unamestr" == 'Darwin' ]]; then
    which greadlink >/dev/null 2>&1
    if [[ "$?" == 1 ]]; then
	brew install coreutils
    fi
    export ANDROID_NDK=`greadlink -f $ndkbuildloc|sed 's/ndk-exec.sh//'|sed 's/ndk-build//'`
fi
echo "Android NDK found at: $ANDROID_NDK"
cd "$(dirname "$0")" # switch to script directory
SCRIPT_ROOT_DIR=`pwd`
INSTALL_DIR=$SCRIPT_ROOT_DIR/install

set +e # hard errors

# Build Lua
mkdir -p build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DWITH_LUA52=ON -DWITH_LUAROCKS=OFF \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DANDROID_STL=none -DLIBRARY_OUTPUT_PATH_ROOT=$INSTALL_DIR  -DCWRAP_CUSTOM_LUA=th \
    -DCMAKE_C_FLAGS="-DDISABLE_POSIX_MEMALIGN" -DARM_TARGETS="armeabi-v7a with NEON" -DNEON_FOUND=true 

make install

cd ..

echo "done"
