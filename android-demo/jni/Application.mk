# Build only ARMv7-A machine code.
APP_ABI := armeabi-v7a
APP_STL :=gnustl_static
APP_CFLAGS += -fopenmp -Wno-error=format-security
APP_LDFLAGS += -fopenmp
