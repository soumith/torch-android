LOCAL_PATH := $(call my-dir)

TH_LIB_DIR=../../../install/libs/${APP_ABI}

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
LOCAL_MODULE := threadsmain
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libthreadsmain.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := threads
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libthreads.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := nn
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libnn.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := nnx
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libnnx.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := image
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libimage.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cutorch
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libcutorch.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := paths
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libpaths.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ppm
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libppm.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := sys
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libsys.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := sundown
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libsundown.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := THC
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libTHC.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := THCUNN
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libTHCUNN.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := THNN
LOCAL_SRC_FILES := ${TH_LIB_DIR}/libTHNN.so
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
