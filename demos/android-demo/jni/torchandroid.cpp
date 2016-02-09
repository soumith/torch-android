#include "torchandroid.h"

#include <android/asset_manager.h>

void android_fopen_set_asset_manager(AAssetManager* manager);
FILE* android_fopen(const char* fname, const char* mode);



static const luaL_Reg lualibs[] =
  {
    { "base",       luaopen_base },
    { "libtorch",   luaopen_libtorch },
    { "nn",         luaopen_libnn },
    { "nnx",        luaopen_libnnx },
    { "image",      luaopen_libimage },
    { NULL,         NULL }
  };

// function to open up all the Lua libraries you declared above
lua_State* openlualibs(lua_State *l)
{
  luaL_openlibs(l);
  const luaL_Reg *lib;
  int ret;
  for (lib = lualibs; lib->func != NULL; lib++)
    {
      lib->func(l);      
      lua_settop(l, 0);
    }
  return l;
}

// function to redirect prints to logcat
static int landroid_print(lua_State* L) {
  int nargs = lua_gettop(L);
  for (int i=1; i <= nargs; i++) {
    if (lua_isstring(L, i)) {
      D(lua_tostring(L, i));
    }
  }
  return 0;
}

static const struct luaL_Reg androidprint [] = {
  {"print", landroid_print},
  {NULL, NULL} /* end of array */
};

extern int luaopen_landroidprint(lua_State *L)
{
  lua_getglobal(L, "_G");
  luaL_setfuncs(L, androidprint, 0);
  lua_pop(L, 1);
}

long android_asset_get_size(const char *name) {
  FILE *fl = android_fopen(name, "r");
  if (fl == NULL)
    return -1;
  fseek(fl, 0, SEEK_END);
  long len = ftell(fl);
  return len;
}

char* android_asset_get_bytes(const char *name) {
  FILE *fl = android_fopen(name, "r");
  if (fl == NULL)
    return NULL;
  fseek(fl, 0, SEEK_END);
  long len = ftell(fl);
  char *buf = (char *)malloc(len);
  fseek(fl, 0, SEEK_SET);
  size_t loaded = fread(buf, 1, len, fl);
  if (loaded != len) {
    fclose(fl);
    return NULL;
  }
  fclose(fl);
  return buf;
}

extern int loader_android (lua_State *L) {
  const char* name = lua_tostring(L, -1);
  name = luaL_gsub(L, name, ".", LUA_DIRSEP);
  char pname[4096];
  char *filebytes;
  long size;
  // try lua/share/lua/5.1/torch.lua
  strlcpy(pname, "lua/share/lua/5.2/", sizeof(pname));
  strlcat(pname, name, sizeof(pname));
  strlcat(pname, ".lua", sizeof(pname));
  size = android_asset_get_size(pname);
  if (size != -1) {
    filebytes = android_asset_get_bytes(pname);
    luaL_loadbuffer(L, filebytes, size, name);
    return 1;    
  }
  // try lua/share/lua/5.1/torch/init.lua
  pname[0] = '\0';
  strlcpy(pname, "lua/share/lua/5.2/", sizeof(pname));
  strlcat(pname, name, sizeof(pname));
  strlcat(pname, "/init.lua", sizeof(pname));
  size = android_asset_get_size(pname);
  if (size != -1) {
    filebytes = android_asset_get_bytes(pname);
    luaL_loadbuffer(L, filebytes, size, name);
    return 1;    
  }
  return 1;
}

lua_State* inittorch(AAssetManager* manager) {
  /* Declare a Lua State, open the Lua State */
  lua_State *L;
  L = luaL_newstate();
  // set the asset manager
  android_fopen_set_asset_manager(manager);
  openlualibs(L);
  luaopen_landroidprint(L);
  // add an android module loader to package.loaders
  lua_getglobal(L, "package");        
  lua_getfield(L, -1, "searchers");
  int numloaders = lua_objlen(L, -1);
  lua_pushcfunction(L, loader_android);
  lua_rawseti(L, -2, numloaders+1);
  lua_pop(L, 1);
  return L;
}


