LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := torchdemo

LOCAL_C_INCLUDES += ../../../install/include

LOCAL_SRC_FILES := torchandroid.cpp torchdemo.cpp android_fopen.c

LOCAL_LDLIBS := -llog -landroid -L ../../../install/lib -L ../../../install/libs/armeabi-v7a  -lluaT -lluajit -lTH  -lTHNN -lnn  -ltorch -lnnx -limage -ltorchandroid -lluaT -lluajit -lTH -lTHNN -lnn  -ltorch -lnnx -limage -ltorchandroid

include $(BUILD_SHARED_LIBRARY)
