if [ $*=="clean" ]; then
	ndk-build clean
	rm -rf assets/lua
	exit
fi

ndk-build
rm -rf assets/lua
mkdir -p assets/lua/share
cp -r ../../share assets/lua/
