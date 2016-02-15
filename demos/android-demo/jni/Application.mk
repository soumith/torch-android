# Build only ARMv7-A machine code.
APP_ABI := armeabi-v7a
APP_STL :=gnustl_static
APP_CFLAGS += -fopenmp
APP_LDFLAGS += -fopenmp
