android update project --path .
ndk-build
if [ $? -ne 0 ]; then
    exit
fi
ant debug
if [ $? -ne 0 ]; then
    exit
fi
adb install -r bin/TorchDemo-debug.apk
if [ $? -ne 0 ]; then
    exit
fi
