# Build only ARM64 machine code.
APP_ABI := arm64-v8a
APP_STL :=gnustl_static
LOCAL_ARM_NEON := true
APP_CFLAGS += -fopenmp
APP_LDFLAGS += -fopenmp
