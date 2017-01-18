LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE := torchdemo

LOCAL_C_INCLUDES += ../../install/include

LOCAL_SRC_FILES := torchdemo.cpp

LOCAL_LDLIBS := -L ../../install/lib -L ../../install/libs/$(APP_ABI)  -lTHCUNN  -lTHC -lcutorch -lnnx -lTHNN -ltorchandroid -llog -landroid

include $(BUILD_SHARED_LIBRARY)
