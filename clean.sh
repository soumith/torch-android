make clean
rm -rf build install
rm -rf install
( cd distro/exe/luajit-rocks/luajit-2.1 && make clean )
( cd distro && ./clean.sh )
