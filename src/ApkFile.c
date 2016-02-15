#include <stdlib.h>
#include <string.h>

#include "luaT.h"
#include "TH.h"
#include "THApkFile.c"

static int torch_ApkFile_new(lua_State *L)
{
  const char *name = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  int isQuiet = luaT_optboolean(L, 3, 0);
  THFile *self = THApkFile_new(name, mode, isQuiet);

  luaT_pushudata(L, self, "torch.ApkFile");
  return 1;
}

static int torch_ApkFile_free(lua_State *L)
{
  THFile *self = luaT_checkudata(L, 1, "torch.ApkFile");
  THFile_free(self);
  return 0;
}

static int torch_ApkFile_isLittleEndianCPU(lua_State *L)
{
  lua_pushboolean(L, THApkFile_isLittleEndianCPU());
  return 1;
}

static int torch_ApkFile_isBigEndianCPU(lua_State *L)
{
  lua_pushboolean(L, !THApkFile_isLittleEndianCPU());
  return 1;
}

static int torch_ApkFile_nativeEndianEncoding(lua_State *L)
{
  THFile *self = luaT_checkudata(L, 1, "torch.ApkFile");
  THApkFile_nativeEndianEncoding(self);
  lua_settop(L, 1);
  return 1;
}

static int torch_ApkFile_littleEndianEncoding(lua_State *L)
{
  THFile *self = luaT_checkudata(L, 1, "torch.ApkFile");
  THApkFile_littleEndianEncoding(self);
  lua_settop(L, 1);
  return 1;
}

static int torch_ApkFile_bigEndianEncoding(lua_State *L)
{
  THFile *self = luaT_checkudata(L, 1, "torch.ApkFile");
  THApkFile_bigEndianEncoding(self);
  lua_settop(L, 1);
  return 1;
}

static int torch_ApkFile_longSize(lua_State *L)
{
  THFile *self = luaT_checkudata(L, 1, "torch.ApkFile");
  THApkFile_longSize(self, lua_tointeger(L, 2));
  lua_settop(L, 1);
  return 1;
}

static int torch_ApkFile___tostring__(lua_State *L)
{
  THFile *self = luaT_checkudata(L, 1, "torch.ApkFile");
  lua_pushfstring(L, "torch.ApkFile on <%s> [status: %s -- mode %c%c]", 
                  THApkFile_name(self),
                  (THFile_isOpened(self) ? "open" : "closed"),
                  (THFile_isReadable(self) ? 'r' : ' '),
                  (THFile_isWritable(self) ? 'w' : ' '));

  return 1;
}
static const struct luaL_Reg torch_ApkFile__ [] = {
  {"isLittleEndianCPU", torch_ApkFile_isLittleEndianCPU},
  {"isBigEndianCPU", torch_ApkFile_isBigEndianCPU},
  {"nativeEndianEncoding", torch_ApkFile_nativeEndianEncoding},
  {"littleEndianEncoding", torch_ApkFile_littleEndianEncoding},
  {"bigEndianEncoding", torch_ApkFile_bigEndianEncoding},
  {"longSize", torch_ApkFile_longSize},
  {"__tostring__", torch_ApkFile___tostring__},
  {NULL, NULL}
};

void torch_ApkFile_init(lua_State *L)
{
  luaT_newmetatable(L, "torch.ApkFile", "torch.File",
                    torch_ApkFile_new, torch_ApkFile_free, NULL);
  
  luaT_setfuncs(L, torch_ApkFile__, 0);
  lua_pop(L, 1);
}
