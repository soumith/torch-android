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
NVCC=`which nvcc`

set +e # hard errors

# Build Lua
# -DANDROID_TOOLCHAIN_NAME=aarch64-linux-android-4.9
rm -fr build install
mkdir -p build install
cd build

export VE="VERBOSE=1"

cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a -DCUDA_ARCH_NAME=Maxwell\
    -DWITH_LUA52=ON -DWITH_LUAROCKS=OFF \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DLIBRARY_OUTPUT_PATH_ROOT=$INSTALL_DIR  -DCUDA_SDK_ROOT_DIR=$CUDA_TOOLKIT_ROOT -DCWRAP_CUSTOM_LUA=th \
    -DCMAKE_C_FLAGS="-DDISABLE_POSIX_MEMALIGN" -DCUDA_TOOLKIT_ROOT_DIR="${CUDA_ANDROID_HOME}/aarch64-linux-androideabi"
echo " ------ CMAKE DONE ------- "

make $VE install


cd ..

echo "done"
