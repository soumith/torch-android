android update project --path .
ndk-build
if [ $? -ne 0 ]; then
    exit
fi
rm -rf assets/lua
cp -r ../../install/share/lua assets/
cp -r ../../install/libs/armeabi-v7a/*.so libs/armeabi-v7a/
ant debug
if [ $? -ne 0 ]; then
    exit
fi
adb install -r bin/facedetector-debug.apk
if [ $? -ne 0 ]; then
    exit
fi
