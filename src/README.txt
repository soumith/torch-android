Torch7 Library.
===============

Torch7 provides a Matlab-like environment for state-of-the-art machine
learning algorithms. It is easy to use and provides a very efficient
implementation, thanks to an easy and fast scripting language (Lua) and a
underlying C implementation.

In order to install Torch7 you can follow these simple instructions, but 
we suggest reading the detailed manual at http://www.torch.ch/manual/install/index

Requirements
============
C/C++ compiler
cmake
gnuplot
git

Optional
========
readline
QT (QT4.8 is now supported)
CBLAS
LAPACK


Installation
============
$ git clone git://github.com/andresy/torch.git
$ cd torch
$ mkdir build
$ cd build

$ cmake .. 
OR
$ cmake .. -DCMAKE_INSTALL_PREFIX=/my/install/path

$make install

Running
=======
$torch
Type help() for more info
Torch 7.0  Copyright (C) 2001-2011 Idiap, NEC Labs, NYU
Lua 5.1  Copyright (C) 1994-2008 Lua.org, PUC-Rio
t7> 

Documentation
=============
The full documentation is installed in /my/install/path/share/torch/html/index.html

Also, http://www.torch.ch/manual/index points to the latest documentation of Torch7.


