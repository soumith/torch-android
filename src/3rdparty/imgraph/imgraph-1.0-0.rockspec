package = "imgraph"
version = "1.0-0"

source = {
   url = "git://github.com/clementfarabet/lua---imgraph",
   tag = "1.0-0"
}

description = {
   summary = "An image/graph library for Torch",
   detailed = [[
This package provides routines to construct graphs on images,
segment them, build trees out of them, and convert them back
to images.
   ]],
   homepage = "https://github.com/clementfarabet/lua---imgraph",
   license = "GPL"
}

dependencies = {
   "torch >= 7.0",
   "sys >= 1.0",
   "xlua >= 1.0",
   "image >= 1.0"
}

build = {
   type = "cmake",
   variables = {
      LUAROCKS_PREFIX = "$(PREFIX)"
   }
}
