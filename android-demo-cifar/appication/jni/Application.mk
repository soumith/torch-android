# Build only ARMv7-A machine code.
APP_ABI := armeabi-v7a
APP_STL :=gnustl_static
LOCAL_ARM_NEON := true
APP_CFLAGS += -fopenmp
APP_LDFLAGS += -fopenmp