#include "torchandroid.h"

#include <android/asset_manager.h>
#include <TH/TH.h>
#include <THApkFile.h>
void android_fopen_set_asset_manager(AAssetManager* manager);
FILE* android_fopen(const char* fname, const char* mode);

static const luaL_reg lualibs[] =
  {
    { "base",       luaopen_base },
    { NULL,         NULL }
  };

// function to open up all the Lua libraries you declared above
static lua_State* openlualibs(lua_State *l)
{
  luaL_openlibs(l);
  const luaL_reg *lib;
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

static const struct luaL_reg androidprint [] = {
  {"print", landroid_print},
  {NULL, NULL} /* end of array */
};

extern int luaopen_landroidprint(lua_State *L)
{
  lua_getglobal(L, "_G");
  luaL_register(L, NULL, androidprint);
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
  // try lua/5.1/torch.lua
  strlcpy(pname, "lua/5.1/", sizeof(pname));
  strlcat(pname, name, sizeof(pname));
  strlcat(pname, ".lua", sizeof(pname));
  size = android_asset_get_size(pname);
  if (size != -1) {
    filebytes = android_asset_get_bytes(pname);
    luaL_loadbuffer(L, filebytes, size, name);
    return 1;    
  }
  // try lua/5.1/torch/init.lua
  pname[0] = '\0';
  strlcpy(pname, "lua/5.1/", sizeof(pname));
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

lua_State* inittorch(AAssetManager* manager, const char* libpath) {
  /* Declare a Lua State, open the Lua State */
  lua_State *L;
  L = lua_open();
  // set the asset manager
  android_fopen_set_asset_manager(manager);
  THApkFile_setAAssetManager((void *) manager);
  openlualibs(L);
  luaopen_landroidprint(L);
  // concat libpath to package.cpath
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "cpath");
  const char* current_cpath = lua_tostring(L, -1);
  lua_pop(L, 1);
  char final_cpath[4096];
  strcpy(final_cpath, libpath);
  strcat(final_cpath, "/?.so;");
  strcat(final_cpath, current_cpath);
  lua_pushstring(L, final_cpath);
  lua_setfield(L, -2, "cpath");
  lua_pop(L, 1); // balance stack

  // add an android module loader to package.loaders
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaders");
  int numloaders = lua_objlen(L, -1);
  lua_pushcfunction(L, loader_android);
  lua_rawseti(L, -2, numloaders+1);
  lua_pop(L, 1);
  return L;
}


