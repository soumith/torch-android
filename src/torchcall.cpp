/*
 * Copyright (C) 2013 Soumith Chintala
 *
 */
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include "torchandroid.h"
#include <assert.h>

extern "C" {

  JNIEXPORT jstring JNICALL
  Java_com_torch_Torch_jni_1call( JNIEnv* env,
                                 jobject thiz,
                                 jobject assetManager,
                                 jstring nativeLibraryDir_,
                                 jstring luaFile_
                                 ) {
    //    D("Hello from C");
    // get native asset manager
    AAssetManager* manager = AAssetManager_fromJava(env, assetManager);
    assert( NULL != manager);
    const char *nativeLibraryDir = env->GetStringUTFChars(nativeLibraryDir_, 0);
    const char *file = env->GetStringUTFChars(luaFile_, 0);

    char buffer[4096]; // buffer for textview output

    D("Torch.call(%s), nativeLibraryDir=%s", file, nativeLibraryDir);

    buffer[0] = 0;

    lua_State *L = inittorch(manager, nativeLibraryDir); // create a lua_State
    assert( NULL != manager);

    // load and run file
    int ret;
    long size = android_asset_get_size(file);
    if (size != -1) {
      char *filebytes = android_asset_get_bytes(file);
      ret = luaL_dobuffer(L, filebytes, size, "main");
    }

    // check if script ran succesfully. If not, print error to logcat
    if (ret == 1) {
      D("Error doing resource: %s:%s\n", file, lua_tostring(L,-1));
      strlcat(buffer, lua_tostring(L,-1), sizeof(buffer));
    }
    else
      strlcat(buffer,
              "Torch script ran succesfully. Check Logcat for more details.",
              sizeof(buffer));
    // destroy the Lua State
    lua_close(L);
    return env->NewStringUTF(buffer);
  }
}
