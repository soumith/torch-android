
#include <TH.h>
#include <luaT.h>

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_Tensor TH_CONCAT_STRING_3(torch., Real, Tensor)
#define image_(NAME) TH_CONCAT_3(image_, Real, NAME)

#ifdef max
#undef max
#endif
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )

#ifdef min
#undef min
#endif
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )

#include "generic/image.c"
#include "THGenerateAllTypes.h"

DLL_EXPORT int luaopen_libimage(lua_State *L)
{
  image_FloatMain_init(L);
  image_DoubleMain_init(L);
  image_ByteMain_init(L);

  luaL_register(L, "image.double", image_DoubleMain__); 
  luaL_register(L, "image.float", image_FloatMain__);
  luaL_register(L, "image.byte", image_ByteMain__);

  return 1;
}
