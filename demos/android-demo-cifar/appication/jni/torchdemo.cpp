/*
 * Copyright (C) 2013 e-lab Purdue
 *
 */
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include "torchandroid.h"
#include <assert.h>

extern "C" {

void PrintTable(lua_State *L, int tableSize, int *tableContents) {
	int i = 0; //increments to store w,x,y,z
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) //while table or integers exist on stack
	{
		if (lua_isnumber(L, -1)) //If number = if x,y,w,h
				{
			tableContents[4 * (tableSize) + i] = lua_tointeger(L, -1);
			i++;
		} else if (lua_istable(L, -1)) // If table = detection. All detections stored in a table of tables
				{
			PrintTable(L, --tableSize, tableContents); //Recursive call to get table elements
		}
		lua_pop(L, 1); //Pop number off stack
	}
}

JNIEXPORT long JNICALL
Java_com_torchandroid_cifardemo_lua_LuaManager_initTorch(JNIEnv *env, jobject thiz, jobject assetManager)
{
	// get native asset manager. This allows access to files stored in the assets folder
	AAssetManager* manager = AAssetManager_fromJava(env, assetManager);
	assert( NULL != manager);

	lua_State *L = NULL;
	L = inittorch(manager);// create a lua_State

	// load and run file
	char file[] = "init-only.lua";
	int ret;
	long size = android_asset_get_size(file);
	if (size != -1) {
		char *filebytes = android_asset_get_bytes(file);
		ret = luaL_dobuffer(L, filebytes, size, "init-only");
		if (ret == 1) {
			D("Torch Error doing resource: %s\n", file);
			D(lua_tostring(L,-1));
		} else {
			D("Torch script ran succesfully.");
		}
	}
	return (long) L;
}
JNIEXPORT void JNICALL
Java_com_torchandroid_cifardemo_lua_LuaManager_destroyTorch(JNIEnv *env, jobject thiz,jlong torchStateLocation)
{
	lua_State *L = (lua_State*) torchStateLocation;
	lua_close(L); //Close lua state.
}

JNIEXPORT int JNICALL
Java_com_torchandroid_cifardemo_lua_LuaManager_getTopResult(JNIEnv *env, jobject thiz, jlong torchStateLocation,
		jint width, jint height, jbyteArray bitmapRGBData) {

	lua_State *L = (lua_State*) torchStateLocation;

	int result = -1;
	int size = width * height;
	THDoubleTensor *testTensor = THDoubleTensor_newWithSize1d(3 * size); //Initialize 1D tensor with size * 3 (R,G,B).
	jdouble *testTensorData = THDoubleTensor_data(testTensor);
	jbyte *inputData;//Initialize tensor to store java byte data from bitmap.
	inputData = (env)->GetByteArrayElements(bitmapRGBData,0);//Get pointer to java byte array region

	for (int i = 0; i < size; i++) {
		//convert Byte value to int by &0xff and to save it as double
		//save R G B seperatly as cifar-10 data set
		testTensorData[i] = inputData[i * 4] & 0xFF;//R
		testTensorData[size + i] = inputData[i * 4 + 1] & 0xFF;//G
		testTensorData[size * 2 + i] = inputData[i * 4 + 2] & 0xFF;//B
	}

	lua_getglobal(L, "getTopOnly");
	luaT_pushudata(L, testTensor, "torch.DoubleTensor");
	if (lua_pcall(L, 1, 1, 0) != 0) {
		//Call function. Print error if call not successful
		__android_log_print(ANDROID_LOG_INFO, "Torchandroid",
				"Error running function: %s", lua_tostring(L, -1));
	} else {
		result = (float) lua_tonumber(L,-1);
		lua_pop(L,1);
		__android_log_print(ANDROID_LOG_INFO, "Torchandroid", "result : %d",result);
	}
	env->ReleaseByteArrayElements(bitmapRGBData, inputData, 0); //Destroy pointer to location in C. Only need java now
	return result;
}

JNIEXPORT float JNICALL
Java_com_torchandroid_cifardemo_lua_LuaManager_testTorchData(JNIEnv *env, jobject thiz, jlong torchStateLocation) {

	lua_State *L = (lua_State*) torchStateLocation;

	float rate = 0.1f;
	//D("Torch 1.");
	lua_getglobal(L,"testTorchData");
	if(lua_pcall(L,0,1,0) != 0)//Call function. Print error if call not successful
	__android_log_print(ANDROID_LOG_INFO, "Torchandroid", "Error running function: %s",lua_tostring(L, -1));
	else {
		rate = (float) lua_tonumber(L,-1);
		lua_pop(L,1);
		__android_log_print(ANDROID_LOG_INFO, "Torchandroid", "rate %f",rate);
	}
	return rate;
}

}
