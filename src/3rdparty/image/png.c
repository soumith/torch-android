
#include <TH.h>
#include <luaT.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PNG_DEBUG 3
#include <png.h>

void abort_(const char * s, ...)
{
  va_list args;
  va_start(args, s);
  vfprintf(stderr, s, args);
  fprintf(stderr, "\n");
  va_end(args);
  abort();
}

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_Tensor TH_CONCAT_STRING_3(torch., Real, Tensor)
#define libpng_(NAME) TH_CONCAT_3(libpng_, Real, NAME)

#include "generic/png.c"
#include "THGenerateAllTypes.h"

DLL_EXPORT int luaopen_libpng(lua_State *L)
{
  libpng_FloatMain_init(L);
  libpng_DoubleMain_init(L);
  libpng_ByteMain_init(L);

  luaL_register(L, "libpng.double", libpng_DoubleMain__);
  luaL_register(L, "libpng.float", libpng_FloatMain__);
  luaL_register(L, "libpng.byte", libpng_ByteMain__);

  return 1;
}
