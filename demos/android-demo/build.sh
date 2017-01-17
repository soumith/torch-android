android update project --path .
ndk-build -v 
if [ $? -ne 0 ]; then
    exit
fi
rm -rf assets/lua
cp -r ../../install/share/lua assets/
cp -r ../../install/libs/armeabi-v7a/*.so libs/armeabi-v7a/
cp -r ../../install/libs/arm64-v8a/*.so libs/arm64-v8a/
ant debug
if [ $? -ne 0 ]; then
    exit
fi
adb install -r bin/TorchDemo-debug.apk
if [ $? -ne 0 ]; then
    exit
fi
