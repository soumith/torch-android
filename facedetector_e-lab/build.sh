android update project --path .
ndk-build
if [ $? -ne 0 ]; then
exit
fi
rm -rf assets/lua/*
mkdir -p assets/lua/
cp -r ../share assets/lua/
ant debug
if [ $? -ne 0 ]; then
exit
fi
adb install -r bin/facedetector-debug.apk
if [ $? -ne 0 ]; then
exit
fi