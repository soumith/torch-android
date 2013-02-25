package = "image"
version = "1.0.1-0"

source = {
   url = "git://github.com/clementfarabet/lua---image",
   tag = "1.0.1-0"
}

description = {
   summary = "An image library for Torch",
   detailed = [[
This package provides routines to load/save and manipulate images
using Torch's Tensor data structure.
   ]],
   homepage = "https://github.com/clementfarabet/lua---image",
   license = "BSD"
}

dependencies = {
   "torch >= 7.0",
   "sys >= 1.0",
   "xlua >= 1.0"
}

build = {
   type = "cmake",
   variables = {
      LUAROCKS_PREFIX = "$(PREFIX)"
   }
}
