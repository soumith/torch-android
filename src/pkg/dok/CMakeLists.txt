SET(luasrc init.lua inline.lua search.lua)

ADD_TORCH_PACKAGE(dok "${src}" "${luasrc}")
