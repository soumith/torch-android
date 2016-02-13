LOCAL_PATH := $(call my-dir)

TH_LIB_DIR=../../../install/libs/${APP_ABI}
LOCAL_C_INCLUDES += ../../install/include

include $(CLEAR_VARS)
LOCAL_MODULE := luaT
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libluaT.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := TH
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libTH.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := torch
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libtorch.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := lua
LOCAL_SRC_FILES := ${TH_LIB_DIR}/liblua.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := torchdemo

#CUDA_ANDROID_ARCH = $(CUDA_ANDROID_HOME)/armv7-linux-androideabi
# CUDA_ANDROID_ARCH = $(CUDA_ANDROID_HOME)/aarch64-linux-androideabi
#LOCAL_LDFLAGS := -L../lib/ -L$(CUDA_ANDROID_ARCH)/lib -L$(CUDA_ANDROID_ARCH)/lib/stubs -DDEBUG
#LOCAL_LDFLAGS := -L../lib/ -L$(CUDA_ANDROID_ARCH)/lib64 -L$(CUDA_ANDROID_ARCH)/lib64/stubs -DDEBUG
LOCAL_LDFLAGS :=
#LOCAL_LDLIBS += -L$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(TARGET_ARCH_ABI) -landroid -llog -ldl
LOCAL_LDLIBS += -landroid -llog -ldl

LOCAL_C_INCLUDES += ../../install/include
LOCAL_SRC_FILES := torchandroid.cpp torchdemo.cpp android_fopen.c
# LOCAL_SHARED_LIBRARIES := lua torch luaT TH threads threadsmain nn nnx image THC cutorch THCUNN THNN paths ppm sys sundown
LOCAL_SHARED_LIBRARIES := lua torch luaT TH

include $(BUILD_SHARED_LIBRARY)
