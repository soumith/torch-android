#ifndef __TORCHANDROID_H__
#define __TORCHANDROID_H__

/*
 * Copyright (C) 2013 Soumith Chintala
 *
 */

#define DEBUG 1

#include <cstdio> // for size_t which is needed b asset_manager.h (wtf android)
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "android_fopen.h"
extern "C" {

#if DEBUG
#include <android/log.h>
#  define  D(x...)  __android_log_print(ANDROID_LOG_INFO,"torchdemo", "%s", x)
#else
#  define  D(...)  do {} while (0)
#endif

#define luaL_dobuffer(L, b, n, s)					\
  (luaL_loadbuffer(L, b, n, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#include "TH/TH.h"
#include "luaT.h"
#include "lualib.h"
#include "lauxlib.h"

  int luaopen_libpaths(lua_State *L);
  int luaopen_libtorch(lua_State *L);
  int luaopen_libnn(lua_State *L);
  int luaopen_libnnx(lua_State *L);
  int luaopen_libimage(lua_State *L);
  
  char* android_asset_get_bytes(const char *name);
  long android_asset_get_size(const char *name);
}
lua_State* inittorch(AAssetManager* m);

#endif // __TORCHANDROID_H__
