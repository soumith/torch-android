ndk-build
if [ $? -ne 0 ]; then
    exit
fi
rm -rf assets/lua/*
cp -r ../share assets/lua/share
ant debug
if [ $? -ne 0 ]; then
    exit
fi
adb install -r bin/TorchDemo-debug.apk
if [ $? -ne 0 ]; then
    exit
fi
