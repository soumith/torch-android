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


Requirements
------------
Android NDK and Android SDK

Samples
--------
* A sample project has been provided in android-demo
* android-demo/jni/torchdemo.cpp is a simple use-case
* android-demo/assets/main.lua is the file that is run
* Vinayak Ghokale from e-lab Purdue (https://github.com/e-lab) contributed a face detector demo, which showcases a fuller use-case.
* That's in the facedetector_e-lab folder. I made some changes to it to load assets etc. from apk as opposed to the sdcard, but it remains untouched otherwise.

Building Torch
--------------
1. open build.sh and modify ANDROID_NDK_TOOLCHAIN_ROOT to your android ndk path.
2. run build script
$ sh build.sh

You can use torch in your android apps. The relevant directories are
* include - include directories
* lib - static libs cross-compiled for armeabi-v7a
* share - lua files


Building Example
----------------
1. Build Torch atleast once using the steps above.
2. [Optional] Connect your android phone in debugging mode,
              to automatically install the apk.
3. Change directory into android-demo folder.
4. Run build script.
$ sh build.sh
5. Run the app TorchDemo on your phone.
