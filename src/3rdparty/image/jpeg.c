
#include <TH.h>
#include <luaT.h>
#include <jpeglib.h>
#include <setjmp.h>

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_Tensor TH_CONCAT_STRING_3(torch., Real, Tensor)
#define libjpeg_(NAME) TH_CONCAT_3(libjpeg_, Real, NAME)

#include "generic/jpeg.c"
#include "THGenerateAllTypes.h"

DLL_EXPORT int luaopen_libjpeg(lua_State *L)
{
  libjpeg_FloatMain_init(L);
  libjpeg_DoubleMain_init(L);
  libjpeg_ByteMain_init(L);

  luaL_register(L, "libjpeg.double", libjpeg_DoubleMain__);
  luaL_register(L, "libjpeg.float", libjpeg_FloatMain__);
  luaL_register(L, "libjpeg.byte", libjpeg_ByteMain__);

  return 1;
}
