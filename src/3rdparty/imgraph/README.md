# imgraph: a package to create/manipulate graphs on images

This package provides standard functions to
create and manipulate edge-weighted graphs 
of images: create a graph, segment it, 
compute its watershed, or its connected
components...

## Install 

1/ Torch7 is required:

Dependencies, on Linux (Ubuntu > 9.04):

``` sh
$ apt-get install gcc g++ git libreadline5-dev cmake wget libqt4-core libqt4-gui libqt4-dev
```

Dependencies, on Mac OS (Leopard, or more), using [Homebrew](http://mxcl.github.com/homebrew/):

``` sh
$ brew install git readline cmake wget qt
```

Then on both platforms:

``` sh
$ git clone https://github.com/andresy/torch
$ cd torch
$ mkdir build; cd build
$ cmake ..
$ make
$ [sudo] make install
```

2/ Once Torch7 is available, install this package:

``` sh
$ [sudo] torch-pkg install imgraph
```

## Use the library

First run torch, and load imgraph:

``` sh
$ torch
``` 

``` lua
> require 'imgraph'
```

Once loaded, tab-completion will help you navigate through the
library:

``` lua
> imgraph. + TAB
imgraph.colorize(           imgraph.connectcomponents(  
imgraph.graph(              imgraph.histpooling(        
imgraph.segmentmst(         imgraph.testme(             
imgraph.watershed(          imgraph.gradient(
```

To get quickly started, run the testme() function:

``` lua
> imgraph.testme()
```

which computes a few things on the famous image of Lena:

![results](http://data.neuflow.org/share/imgraph-testme.jpg)
