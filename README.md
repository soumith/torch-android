##########################################
## Torch-7 for Android                  ##
##########################################
Torch7 provides a Matlab-like environment for state-of-the-art machine
learning algorithms. It is easy to use and provides a very efficient
implementation, thanks to an easy and fast scripting language (Lua) and a
underlying C implementation.

Modified to be compiled and used with Android

Features
--------
* Loading of lua packages from the apk directly.
* This is done by writing a custom package.loader
  Reference: http://www.lua.org/manual/5.1/manual.html#pdf-package.loaders
  The loader is in torchandroid.cpp as loader_android
* torchandroid.h and torchandroid.cpp give lots of helper functions to make life easier
  * Print function overriden to redirect to logcat (only handles strings for now)
  * Function to get apk assets as bytes (very useful)
* Full support for ffi and shared libraries

`torch.load` now takes three additional modes: `apkbinary32`, `apkbinary64`, `apkascii`. One can store model files in the `assets` folder and use these modes to load them. If the model was saved on a 64-bit machine, use `apkbinary64`, if it was saved on a 32-bit machine, use `apkbinary32`.


Requirements
------------
For CUDA-enabled version: NVIDIA CodeWorks for Android: https://developer.nvidia.com/codeworks-android. 
* NOTE: CodeWorks 1R5 does not have CUDA! You need to install 1R5 and then CUDA from 1R4.
  
For CPU-only version : Android NDK (13b) and Android SDK 

* NOTE (Nov 2016): Android NDK v13b currently is required for NEON, even if you are building CUDA version with CodeWorks.
This is due to some NDK bugs fixed in v13b - CodeWorks has 12b. NDK will only be used to build Lua JIT.
  1. Get it here: https://dl.google.com/android/repository/android-ndk-r13b-linux-x86_64.zip.
  2. Extract it under ~/NVPACK, next to 12b that comes with CodeWorks.
  3. Change NVPACK environvent to point to that NDK (see sample in ./.bashrc-android)

Samples
--------
* Three sample projects has been provided in demos/
* demos/android-demo/jni/torchdemo.cpp is a simple use-case
* demos/android-demo/assets/main.lua is the file that is run
* demos/android-demo-cifar showcases classifying Camera inputs (or images from gallery) into one of 10 CIFAR-10 categories.
* Vinayak Ghokale from e-lab Purdue (https://github.com/e-lab) contributed a face detector demo, which showcases a fuller use-case (demos/facedetector_e-lab ).

Building Torch Libraries and Java class. 
--------------
If on ubuntu, install the following packages: `sudo apt-get install libx32gcc-4.8-dev libc6-dev-i386`
Default is to build with CUDA - so make sure you installed NVIDIA CodeWorks for Android and its nvcc is in your PATH.
Otherwise, set WITH_CUDA=OFF in build.sh

0. git submodule update --init --recursive
1. Optionally, open build.sh and modify ARCH (to match your device architecture) and WITH_CUDA variables.
2. run build script:
3 ./build.sh 

You can use torch in your android apps. The relevant directories are
* install/include - include directories
* install/libs/$APP_ABI - static libs cross-compiled for your APP_ABI
* install/share/lua - lua files


Building Android Demo App 
----------------
1. Build Torch-Android atleast once using the steps above.
2. [Optional] Connect your android phone in debugging mode,
              to automatically install the apk.
3. Change directory into demos/android-demo folder.
4. Run build script.
$ ./build.sh
5. Run the app TorchDemo on your phone.
