# Build only ARMv7-A machine code.
APP_ABI := arm64-v8a
APP_CFLAGS += -fopenmp
APP_LDFLAGS += -fopenmp
