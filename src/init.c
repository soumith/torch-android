
#include <TH.h>
#include <luaT.h>

#include "ApkFile.c"

DLL_EXPORT int luaopen_libtorchandroid(lua_State *L)
{
  
  torch_ApkFile_init(L);

  return 1;
}
