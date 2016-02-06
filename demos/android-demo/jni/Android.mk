LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := luaT
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libluaT.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := TH
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libTH.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := torch
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libtorch.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := lua
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/liblua.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := threadsmain
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libthreadsmain.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := threads
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libthreads.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := nn
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libnn.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := nnx
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libnnx.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := image
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libimage.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cutorch
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libcutorch.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := paths
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libpaths.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ppm
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libppm.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := sys
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libsys.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := sundown
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libsundown.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := THC
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libTHC.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := THCUNN
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libTHCUNN.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := THNN
LOCAL_SRC_FILES := ../../../install/libs/arm64-v8a/libTHNN.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := torchdemo
CUDA_ANDROID_ARCH = $(CUDA_ANDROID_HOME)/aarch64-linux-androideabi
LOCAL_LDFLAGS := -L../lib/ -L$(CUDA_ANDROID_ARCH)/lib64 -L$(CUDA_ANDROID_ARCH)/lib64/stubs -DDEBUG
LOCAL_LDLIBS += -L$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(TARGET_ARCH_ABI) -lgnustl_static -landroid -llog
LOCAL_C_INCLUDES += ../../install/include
LOCAL_SRC_FILES := torchandroid.cpp torchdemo.cpp android_fopen.c
LOCAL_SHARED_LIBRARIES := lua torch luaT TH threads threadsmain nn nnx image THC cutorch THCUNN THNN paths ppm sys sundown
include $(BUILD_SHARED_LIBRARY)

