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


  void PrintTable(lua_State *L, int tableSize, int *tableContents)
  {
    int i = 0; //increments to store w,x,y,z
    lua_pushnil(L);
    while(lua_next(L, -2) != 0)  //while table or integers exist on stack
      {
	if(lua_isnumber(L, -1)) //If number = if x,y,w,h
	  {
	    tableContents[4*(tableSize)+i] = lua_tointeger(L,-1);
	    i++;
	  }
	else if(lua_istable(L, -1)) // If table = detection. All detections stored in a table of tables
	  {
	    PrintTable(L,--tableSize, tableContents); //Recursive call to get table elements
	  }
	lua_pop(L,1); //Pop number off stack
      }
  }

  static int parse(lua_State *L)
  {
    const char* id = luaT_typenameid(L, "torch.FloatTensor"); //Get float
    THFloatTensor *tensor = (THFloatTensor*) luaT_checkudata(L, 1, id); //Check if float
    float *input_data = THFloatTensor_data(tensor); //Pointer to tensor region

    float threshold = lua_tonumber(L, 2); //Threshold sent by lua
    int table_blobs = 3;
    int idx = lua_objlen(L, 3) + 1;
    float scale = lua_tonumber(L, 4);  //Which scale was this called for?

    // loop over pixels
    int x,y;
    for (y=0; y<tensor->size[0]; y++) {
      for (x=0; x<tensor->size[1]; x++) {
	float val = THFloatTensor_get2d(tensor, y, x);
	if (val > threshold) {
	  // entry = {}
	  lua_newtable(L);
	  int entry = lua_gettop(L);

	  // entry[1] = x
	  lua_pushnumber(L, x);
	  lua_rawseti(L, entry, 1);

	  // entry[2] = y
	  lua_pushnumber(L, y);
	  lua_rawseti(L, entry, 2);

	  // entry[3] = scale
	  lua_pushnumber(L, scale);
	  lua_rawseti(L, entry, 3);

	  // blobs[idx] = entry; idx = idx + 1
	  lua_rawseti(L, table_blobs, idx++);
	}
      }
    }
    return 1;
  }

  JNIEXPORT long JNICALL
  Java_com_torchandroid_facedemo_CameraClass_initTorch(JNIEnv *env, jobject thiz, jobject assetManager)
  {
    // get native asset manager. This allows access to files stored in the assets folder
    AAssetManager* manager = AAssetManager_fromJava(env, assetManager);
    assert( NULL != manager);

    lua_State *L = NULL;
    L = inittorch(manager); // create a lua_State

    // load and run file
    char file[] = "main.lua"; 
    int ret;
    long size = android_asset_get_size(file);
    if (size != -1) {
      char *filebytes = android_asset_get_bytes(file);
      ret = luaL_dobuffer(L, filebytes, size, "main");
      if (ret == 1) {
	D("Torch Error doing resource: %s\n", file);
	D(lua_tostring(L,-1));
      } else {
	D("Torch script ran succesfully.");
      }
    }
    lua_register(L,"parse",parse); //This function is used by main.lua.

    return (long) L;
  }


  JNIEXPORT float JNICALL
  Java_com_torchandroid_facedemo_CameraClass_callTorch(JNIEnv *env, jobject thiz, jlong torchStateLocation,
						       jint width, jint height, jbyteArray NV21FrameData, jintArray outPixels) {

    lua_State *L = (lua_State*) torchStateLocation;

    float netProfiler = 0;

    THFloatTensor *testTensor = THFloatTensor_newWithSize1d(1280*768);  //Initialize 1D tensor.
    jbyte *testTensor_data; //Initialize tensor to store java byte data from camera.
    testTensor_data = (env)->GetByteArrayElements(NV21FrameData,0); //Get pointer to java byte array region
    int imSize = 1280*768; //Define number of pixels

    jfloat *poutPixels = THFloatTensor_data(testTensor); //Torch tensor type to int
    jint *output = env->GetIntArrayElements(outPixels, 0); //Get java int array region for output

    //This loop ignores U and V channels. Network doesn't use them
    //Cam data comes like so - YYYYYY ... imSize times.... YYYYY UVUVUVUVUVUV.... <- ignore these
    for(int i = 0; i < imSize; i++)
      {
	output[i] = 0;
	poutPixels[i] = testTensor_data[i] & 0xFF;
      }

    int tableSize = 0; //Holds number of detections

    int *fill;

    lua_getglobal(L,"getDetections");

    lua_getglobal(L,"network");
    luaT_pushudata(L,testTensor,"torch.FloatTensor"); //Push tensor to lua stack
    lua_pushnumber(L,width);
    lua_pushnumber(L,height);
    if(lua_pcall(L,4,3,0) != 0) //Call function. Print error if call not successful
      __android_log_print(ANDROID_LOG_INFO, "Torchandroid", "Error running function: %s",lua_tostring(L, -1));

    else{

      netProfiler = (float) lua_tonumber(L,-1);
      lua_pop(L,1);

      tableSize = lua_tointeger(L,-1);  //Get #detections from stack
      lua_pop(L,1);
      if(tableSize != 0)  //Extract x,y,w,h for each detection
	{
	  fill = (int*) malloc(4*tableSize*sizeof(int)); //Holds detections
	  PrintTable(L,tableSize,fill);

	}
    }

    if(tableSize != 0)
      {
	int center[2] = {0};
	for(int i = 0; i < 4*tableSize; i+=4)
	  {
	    int x = fill[i];
	    int y = fill[i+1];
	    int w = fill[i+2];
	    int h = fill[i+3];

	    for(int j = i+4; j < 4*tableSize; j+=4)
	      {
		center[0] = fill[j]+fill[j+2]*0.5; //x center
		center[1] = fill[j+1]+fill[j+3]*0.5; //y center
		if(((center[0] <= (x+w)) && (center[0] >= x)) && ((center[1] <= (y+h)) && (center[1] >= y)))
		  {
		    fill[j+2] = 0;
		  }
	      }

	  }
      }

    if(tableSize != 0)
      {
	int tempnum2 = 2*1280;
	int tempnum3 = 3*1280;

	int jlim = 0; //Define to prevent computation of loop control for each iteration. Efficiency FTW
	int tempnum1 = 0;
	int center[2] = {0};  //Holds center of box xy

	for(int i = 0; i < 4*tableSize; i+=4)
	  {

	    if(fill[i+2] == 0)
	      continue;

	    int x = fill[i];
	    int y = fill[i+1];
	    int w = fill[i+2];
	    int h = fill[i+3];

	    int tempnum2 = h*2*1280;
	    int tempnum3 = h*3*1280;

	    __android_log_print(ANDROID_LOG_INFO, "Torchandroid", "x = %u y = %u w = %u h = %u",x,y,w,h);


	    jlim = ((y-1)*1280+x+w);
	    tempnum1 = 1280*(h-1);
	    //Assign output pixels red color. 4 byte - ARGB. x,y from network in 2D. Convert to 1 D
	    for(int j = (y-1)*1280+x; j < jlim; j++)  //This loop does top and bottom lines of box
	      {
		output[j-1280] = 0xFFFF0000;
		output[j+1280] = 0xFFFF0000;
		output[j] = 0xFFFF0000;
		output[j+tempnum1] = 0xFFFF0000;
		output[j+tempnum1-1280] = 0xFFFF0000;
		output[j+tempnum1+1280] = 0xFFFF0000;
	      }

	    jlim = (((y-1)*1280+x)+((1280*h)+w));
	    for(int j = (y-1)*1280+x; j < jlim; j+=1280) //This loop does left and right of box
	      {
		output[j+1] = 0xFFFF0000;
		output[j-1] = 0xFFFF0000;
		output[j] = 0xFFFF0000;
		output[j+w] = 0xFFFF0000;
		output[j+w+1] = 0xFFFF0000;
		output[j+w-1] = 0xFFFF0000;
	      }
	  }
      }

    env->ReleaseByteArrayElements(NV21FrameData, testTensor_data, 0); //Destroy pointer to location in C. Only need java now
    env->ReleaseIntArrayElements(outPixels, output, 0); //Same as above here
    return netProfiler;
  }


  JNIEXPORT void JNICALL
  Java_com_torchandroid_facedemo_CameraClass_destroyTorch(JNIEnv *env, jobject thiz,jlong torchStateLocation)
  {
    lua_State *L = (lua_State*) torchStateLocation;
    lua_close(L); //Close lua state.
  }

}
