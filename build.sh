
# Modify this variable to point to your android ndk root
export ANDROID_NDK_TOOLCHAIN_ROOT=/usr/local/Cellar/android-ndk/r8e

####################################################
# You do not need to modify anything below this line
####################################################
export ANDROID_NDK=$ANDROID_NDK_TOOLCHAIN_ROOT
INSTALL_DIR=`pwd`
cd src
rm -f build/CMakeCache.txt
mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake .. -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DANDROID_STL=none
make install
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
