SET(src)
SET(luasrc init.lua types.lua)
  
ADD_TORCH_PACKAGE(wrap "${src}" "${luasrc}")

