LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := torchdemo

LOCAL_C_INCLUDES += ../../include/torch

LOCAL_SRC_FILES := torchandroid.cpp torchdemo.cpp android_fopen.c

LOCAL_LDLIBS := -llog -landroid -L../../lib/ -lluaT -ltorch-lua-static -lTH  -lnn  -ltorch -lnnx -limage -limgraph -lluaT -ltorch-lua-static -lTH -lnn  -ltorch -lnnx -limage -limgraph -ldl

include $(BUILD_SHARED_LIBRARY)
